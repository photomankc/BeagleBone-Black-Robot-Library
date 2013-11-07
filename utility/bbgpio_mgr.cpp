#include <iostream>
#include "bbgpio_mgr.h"

// Static class data
unordered_map<int, GPIO_PinData>	BBGPIO_Mgr::ms_pinStore;
int 				BBGPIO_Mgr::ms_instanceID=0;

/** @brief Construct a default BBGPIO_Mgr object
  *
  */
BBGPIO_Mgr::BBGPIO_Mgr()
{
	init();
}



/** @brief Construct a BBGPIO_Mgr object with the request operation mode
  *
  * \param mode Integer, [BGPIO_MGR_MODEFS | BBGPIO_MGR_MODEMEM]
  */
 BBGPIO_Mgr::BBGPIO_Mgr(int mode)
{
    init();

    if (mode == BBGPIO_MGR_MODEFS)
        m_mode = BBGPIO_MGR_MODEFS;
    else
        m_mode = BBGPIO_MGR_MODEMEM;
}



/** @brief Destructor
  *
  */
BBGPIO_Mgr::~BBGPIO_Mgr()
{
	ms_instanceID--;
	if(ms_instanceID < 1)
	{
		deleteAll();
		#ifdef DEBUG
		cout << "  Destroyed BBGPIO_Mgr ID: " << m_id << endl;
		#endif
	}
}



void BBGPIO_Mgr::init()
{
    ms_instanceID++;
    m_id = ms_instanceID;
    m_mode = BBGPIO_MGR_MODEFS;
    m_lastErr = 0;
    ms_pinStore.reserve(BBGPIO_MGR_PINCNT);

	#ifdef DEBUG
	cout << "  Created BBGPIO_Mgr ID: " << m_id << endl;
	#endif
}


/** @brief Create a GPIO_Pin objec for the requested header pin/GPIO number.
  *
  * (Returns a pointer to a GPIO_Pin object for the request header pin position on the BeagleBoard expansion headers)
  *
  * \param headerNum Integer GPIO number (Uses ENUM to map PX_XX to the correct GPIO number
  * \return GPIO_Pin pointer or NULL on failure.
  */
GPIO_Pin* BBGPIO_Mgr::aquirePin(int headerNum)
{;
	int result = 0;

	if (m_mode == BBGPIO_MGR_MODEFS)
	{	// Using SYSFS access methods
		if (ms_pinStore.count(headerNum) == 0)
		{
			// TODO - Read the pinmux file and determine if the requested pin is muxed as a GPIO pin.
			//				If not then return NULL, if so continue.

			// If this is the first time the pin has been requested then create a new pin and
			// maintain it in class static container.  Activate the pin and set it to a safe default of input.
			GPIO_PinData pinData;
			pinData.m_pPin = new FSGPIO_Pin(headerNum);
			pinData.m_refCnt = 0;
			pinData.m_kernPin = lookupKernPin(headerNum);

			if(pinData.m_kernPin == -1)
			{
			    // if the kernel pin number can't be located then it's not a valid pin
			    // delete the object and return NULL
			    delete pinData.m_pPin;
				m_lastErr = GPIO_GENERR;
				return NULL;
			}

			result = pinData.m_pPin->activate();
			if (result < 0)
			{
				// If activation fails, note the error, delete the object and return NULL
				delete pinData.m_pPin;
				m_lastErr = result;
				return NULL;
			}

			result = pinData.m_pPin->set_dir(INPUT);
			if (result < 0)
			{
				// If the direction can't be set, note the error, delete, and return NULL
				delete pinData.m_pPin;
				m_lastErr = result;
				return NULL;
			}

			// Crude reference tracking for releasing pins.  If the pin is checked out multiple times
			// then this number will be greater than one.
			pinData.m_refCnt++;
			ms_pinStore[headerNum] = pinData;

			#ifdef DEBUG
			cout << "  Aquired GPIO_Pin[" << headerNum << "] : [" << pinData.m_kernPin << "]"<< endl;
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
			cout << "  Aquired GPIO_Pin[" << headerNum << "] Ref:" << pinData.m_refCnt << endl;
			#endif
			return pinData.m_pPin;
		}
	}
	else
	{
		// TODO: For future Memory Mapped pins
		#ifdef DEBUG
		cout << "  How did we get here?" << endl;
		#endif
		return NULL;
	}
}



/** @brief Release control of GPIO pin.
  *
  * \param Pointer to the GPIO pin object to release.
  * \return Zero on success or negative value on failure.
  */
int BBGPIO_Mgr::releasePin(GPIO_Pin* p_Pin)
{
	// Protect against a NULL pointer
	if (p_Pin ==  NULL)
		return GPIO_GENERR;

	// If we got a pin we created then decrement the reference count
	int gpioNum = p_Pin->get_GPIONum();
	if (ms_pinStore.count(gpioNum) == 0)
	{
		#ifdef DEBUG
		cout << "  GPIO_Pin[" << gpioNum << "] - No such pin aquired!" << endl;
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
			cout << "  Released GPIO_Pin[" << gpioNum << "]" << endl;
			#endif // DEBUG
		}
		else
		{
			#ifdef DEBUG
			cout << "  Released reference to GPIO_Pin[" << gpioNum << "]" << endl;
			#endif // DEBUG
		}
	}
	return 0;
}



/** @brief Force removal of GPIO_Pin object for the indicated BB header pin.
  *
  * \param  headerNum Integer, either the board header_pin number or the GPIO number for the pin to delete
  * \return Integer, 0 for success | negative number
  *
  */
int BBGPIO_Mgr::deletePin(int headerNum)
{
    if (ms_pinStore.count(headerNum))
    {
        GPIO_PinData &pinData = ms_pinStore[headerNum];
        delete pinData.m_pPin;
        ms_pinStore.erase(headerNum);
        #ifdef DEBUG
        cout << "  Deleted GPIO_Pin[" << headerNum << "]"<< endl;
        #endif
        return 0;
    }

    return GPIO_GENERR;
}



/** @brief Force removal of all GPIO_Pin objects stored in the manager.
  *
  * \return Integer, number of pin objects deleted.
  *
  */
int BBGPIO_Mgr::deleteAll()
{
    int delCnt = 0;

    for(int i=0; i<BBGPIO_MGR_MAXGPIO; i++)
    {
        if (ms_pinStore.count(i))
        {
            GPIO_PinData &pinData = ms_pinStore[i];
            delete pinData.m_pPin;
            ms_pinStore.erase(i);
            delCnt++;
            #ifdef DEBUG
            cout << "  Deleted GPIO_Pin[" << i << "]"<< endl;
            #endif
        }
    }

    return delCnt;
}



/** @brief Lookup the kernel pin number for a given GPIO number.
  *
  * (documentation goes here)
  */
int BBGPIO_Mgr::lookupKernPin(int gpioNum)
{
    int result = 0;

    // Check for proper range.  Return error if out of range.
    if (gpioNum < 0 or gpioNum > BBGPIO_MGR_MAXGPIO)
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
  */
int BBGPIO_Mgr::getLastErr()
{
    int result = m_lastErr;
    m_lastErr = 0;
    return result;
}


