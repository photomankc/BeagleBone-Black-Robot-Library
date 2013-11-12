#ifndef GPIO_MGR_H
#define GPIO_MGR_H
#define DEBUG

/** BBGPIO_Mgr Class.
 *  Class to handle accessing and tracking use of BeagleBone GPIO pins.  The
 *  manager will create pin objects and maintain them in internal storage.
 *  Multiple manager objects can be created where needed but all will share
 *  the same storage.  The manager will check if the pin requested is muxed 
 *  properly and is not claimed by any other process.  All pin objects contained 
 *  are deleted when the final manager object goes out of scope.
 *
 *   @author     Kyle Crane
 *   @version    0.5.0
 */

// TODO: Rename this from BBGPIO_Mgr to just GPIO_Mgr.  This should be able to
// work on any linux system that can support memory mapped I/O and SYS_FS I/O.

#include <unordered_map>
#include <string.h>
#include <iostream>
#include "gpio_pin_defs.h"
#include "gpio_pin.h"
#include "fsgpio_pin.h"


struct GPIO_PinData
{
    GPIO_Pin* m_pPin;   /**<GPIO_Pin pointer, pointer to GPIO_Pin object  */
    int m_refCnt;       /**<Integer, Reference count for this pin */
    int m_kernPin;      /**<Integer, Kernel pin number for this GPIO_Pin */
};

class BBGPIO_Mgr
{
    private:
        int m_mode;     /**<Integer, pin mode for pins that  */
        int m_id;       /**<Integer, manager ID number */
        int m_lastErr;  /**<Integer, error code from last operation */

        static int  ms_instanceID;
        static std::unordered_map<int, GPIO_PinData> ms_pinStore;

    public:
        BBGPIO_Mgr();
        BBGPIO_Mgr(int mode);
        virtual ~BBGPIO_Mgr();

        int         getID() {return m_id;};
        GPIO_Pin*   aquirePin(int headerNum);
        int         releasePin(GPIO_Pin *p_Pin);
        int         deletePin(int headerNum);
        int         deleteAll();
        int         getLastErr();

    private:
        int     lookupKernPin(int gpioNum);
        void    init();
};

// Static class data
std::unordered_map<int, GPIO_PinData>	BBGPIO_Mgr::ms_pinStore;
int 				BBGPIO_Mgr::ms_instanceID=0;

/** @brief Construct a default BBGPIO_Mgr object
 *
 */
inline BBGPIO_Mgr::BBGPIO_Mgr()
{
	init();
}



/** @brief Construct a BBGPIO_Mgr object with the requested operation mode
 *
 * \param mode Integer, [BGPIO_MGR_MODEFS | BBGPIO_MGR_MODEMEM]
 */
inline BBGPIO_Mgr::BBGPIO_Mgr(int mode)
{
    init();
    
    if (mode == GPIO_MGR_MODEFS)
        m_mode = GPIO_MGR_MODEFS;
    else
        m_mode = GPIO_MGR_MODEMEM;
}



/** @brief Destructor
 *
 */
inline BBGPIO_Mgr::~BBGPIO_Mgr()
{
	ms_instanceID--;
	if(ms_instanceID < 1)
	{
		deleteAll();
#ifdef DEBUG
		std::cout << "  Destroyed BBGPIO_Mgr ID: " << m_id << std::endl;
#endif
	}
}



inline void BBGPIO_Mgr::init()
{
    ms_instanceID++;
    m_id = ms_instanceID;
    m_mode = GPIO_MGR_MODEFS;
    m_lastErr = 0;
    ms_pinStore.reserve(GPIO_MGR_PINCNT);
    
#ifdef DEBUG
	std::cout << "  Created BBGPIO_Mgr ID: " << m_id << std::endl;
#endif
}


/** @brief Create a GPIO_Pin objec for the requested header pin/GPIO number.
 *
 *  Returns a pointer to a GPIO_Pin object for the request header pin position
 *  on the BeagleBoard expansion headers
 *
 * @param headerNum: Integer GPIO number (maps PX_XX to the GPIO number)
 * @return GPIO_Pin*: Pointer to pin or NULL on failure.
 */
inline GPIO_Pin* BBGPIO_Mgr::aquirePin(int headerNum)
{
	int result = 0;
    
	if (m_mode == GPIO_MGR_MODEFS)
	{	// Using SYSFS access methods
		if (ms_pinStore.count(headerNum) == 0)
		{
			// TODO - Read the pinmux file and determine if the requested pin is
            // muxed as a GPIO pin. If not then return NULL, if so continue.
            
			// If this is the first time the pin has been requested then create
            // a new pin and maintain it in class static container.  Activate
            // the pin and set it to a safe default of input.
			GPIO_PinData pinData;
			pinData.m_pPin = new FSGPIO_Pin(headerNum);
			pinData.m_refCnt = 0;
			pinData.m_kernPin = lookupKernPin(headerNum);
            
			if(pinData.m_kernPin == -1)
			{
			    // if the kernel pin number can't be located then it's not a
                // valid pin delete the object and return NULL.
			    delete pinData.m_pPin;
				m_lastErr = GPIO_GENERR;
				return NULL;
			}
            
			result = pinData.m_pPin->activate();
			if (result < 0)
			{
				// If activation fails, note the error, delete the object and
                // return NULL
				delete pinData.m_pPin;
				m_lastErr = result;
				return NULL;
			}
            
			result = pinData.m_pPin->set_dir(GPIO_IN);
			if (result < 0)
			{
				// If the direction can't be set, note the error, delete, and
                // return NULL
				delete pinData.m_pPin;
				m_lastErr = result;
				return NULL;
			}
            
			// Crude reference tracking for releasing pins.  If the pin is
            // checked out multiple times then this number will be greater than
            // one.
			pinData.m_refCnt++;
			ms_pinStore[headerNum] = pinData;
            
#ifdef DEBUG
			std::cout << "  Aquired GPIO_Pin[" << headerNum << "] : [" << pinData.m_kernPin << "]"<< std::endl;
#endif
			return pinData.m_pPin;
		}
		else
		{
			// If the pin has been previously aquired then return a pointer to the existing
			// pin object and increment the reference count.
			GPIO_PinData &pinData = ms_pinStore[headerNum];
			pinData.m_refCnt++;
#ifdef DEBUG
			std::cout << "  Aquired GPIO_Pin[" << headerNum << "] Ref:" << pinData.m_refCnt << std::endl;
#endif
			return pinData.m_pPin;
		}
	}
	else
	{
		// TODO: For future Memory Mapped pins
#ifdef DEBUG
		std::cout << "  How did we get here?" << std::endl;
#endif
		return NULL;
	}
}



/** @brief Release control of GPIO pin.
 *
 * @param Pointer to the GPIO pin object to release.
 * @return int: 0 | negative value on error.
 */
inline int BBGPIO_Mgr::releasePin(GPIO_Pin* p_Pin)
{
	// Protect against a NULL pointer
	if (p_Pin ==  NULL)
		return GPIO_GENERR;
    
	// If we got a pin we created then decrement the reference count
	int gpioNum = p_Pin->get_GPIONum();
	if (ms_pinStore.count(gpioNum) == 0)
	{
#ifdef DEBUG
		std::cout << "  GPIO_Pin[" << gpioNum << "] - No such pin aquired!" << std::endl;
#endif
		return GPIO_GENERR;
	}
	else
	{
		GPIO_PinData &pinData = ms_pinStore[gpioNum];
		pinData.m_refCnt--;
		if(pinData.m_refCnt == 0)
		{
			// If that was the last reference then delete the pin.
			delete pinData.m_pPin;
			ms_pinStore.erase(gpioNum);
#ifdef DEBUG
			std::cout << "  Released GPIO_Pin[" << gpioNum << "]" << std::endl;
#endif // DEBUG
		}
		else
		{
#ifdef DEBUG
			std::cout << "  Released reference to GPIO_Pin[" << gpioNum << "]" << std::endl;
#endif // DEBUG
		}
	}
	return 0;
}



/** @brief Force removal of GPIO_Pin object for the indicated BB header pin.
 *
 * @param  headerNum: The GPIO number for the pin to delete
 * @return int: 0 | negative number for error.
 *
 */
inline int BBGPIO_Mgr::deletePin(int headerNum)
{
    if (ms_pinStore.count(headerNum))
    {
        GPIO_PinData &pinData = ms_pinStore[headerNum];
        delete pinData.m_pPin;
        ms_pinStore.erase(headerNum);
#ifdef DEBUG
        std::cout << "  Deleted GPIO_Pin[" << headerNum << "]"<< std::endl;
#endif
        return 0;
    }
    
    return GPIO_GENERR;
}



/** @brief Force removal of all GPIO_Pin objects stored in the manager.
 *
 * @return int: Number of pin objects deleted.
 */
inline int BBGPIO_Mgr::deleteAll()
{
    int delCnt = 0;
    
    for(int i=0; i<GPIO_MGR_MAXGPIO; i++)
    {
        if (ms_pinStore.count(i))
        {
            GPIO_PinData &pinData = ms_pinStore[i];
            delete pinData.m_pPin;
            ms_pinStore.erase(i);
            delCnt++;
#ifdef DEBUG
            std::cout << "  Deleted GPIO_Pin[" << i << "]"<< std::endl;
#endif
        }
    }
    
    return delCnt;
}



/** @brief Lookup the kernel pin number for a given GPIO number.
 *
 * (documentation goes here)
 */
inline int BBGPIO_Mgr::lookupKernPin(int gpioNum)
{
    int result = 0;
    
    // Check for proper range.  Return error if out of range.
    if (gpioNum < 0 or gpioNum > GPIO_MGR_MAXGPIO)
        return GPIO_GENERR;
    
    // Lookup the kernel pin number using the GPIO number
    // as the index.  Return error if the furnished GPIO number
    // has no kernel pin assigned.
    result = KERNEL_PIN_CONV[gpioNum][0];
    if (result == -1)
        return GPIO_GENERR;
    
    return result;
}



/** @brief Return the last error code stored.
 *
 *  @return int: Error code.
 */
inline int BBGPIO_Mgr::getLastErr()
{
    int result = m_lastErr;
    m_lastErr = 0;
    return result;
}

#endif //GPIO_MGR_H
