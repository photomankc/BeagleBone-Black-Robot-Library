#ifndef FSGPIO_PIN_H
#define FSGPIO_PIN_H

/** @brief Class to manage a single GPIO pin interface using SYS_FS interface.
 *
 *  Class to represent BeagleBone GPIO pins based on SYSFS access.  Allows 
 *  access to functions of GPIO available through the
 *  SYSFS kernel interface in /sys/class/gpio/.  Allows seting the input/output
 *  direction and value as well as interrupt
 *  setting (future).
 *
 *   @author     Kyle Crane
 *   @version    0.9.0
 */

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "gpio_pin.h"


class FSGPIO_Pin : public GPIO_Pin
{
public:
    FSGPIO_Pin();
    FSGPIO_Pin(int num);
    virtual ~FSGPIO_Pin();

    int     connectGPIO(int num);
    int     activate();
    int     activate(int num);
    int     deactivate();
    void    set(int val);
    int     get();
    int     set_dir(int dir);
    int     get_dir();

    
private:
    
    int     m_dirFd;     /**< File handle for the pin value file. */
    int		m_valFd;     /**< File handle for GPIO pin. */
};






/** @brief Default constructor.
 *
 *  Creates an unattached GPIO object
 */
inline FSGPIO_Pin::FSGPIO_Pin()
{
	m_GPIONum = -1;
	m_sGPIONum = "";
	m_valFd = -1;
	m_dirFd = -1;
    m_dir = GPIO_IN;
	m_active = 0;
}


/** @brief Creates an object attached to the specified kernel GPIO pin number
 *
 *	@param num: integer The GPIO number to attach to.
 */
inline FSGPIO_Pin::FSGPIO_Pin(int num)
{
	m_GPIONum = -1;
	m_sGPIONum = "";
	m_active = 0;
	m_valFd = -1;
	m_dirFd = -1;
    m_dir = GPIO_IN;
    
	// Attach this pin to the requested GPIO number
	connectGPIO(num);
}


/** @brief Destructor
 *
 */
inline FSGPIO_Pin::~FSGPIO_Pin()
{
	FSGPIO_Pin::deactivate();
}


/** @brief Connect the GPIO object to the specified GPIO pin number.
 *
 *	@param num: GPIO number to connect this object to.
 *	@return int: result code.  Negative numbers represent failure
 */
inline int FSGPIO_Pin::connectGPIO(int num)
{
    // If this pin was previously exported then unexport it.
    if (m_GPIONum > 0 and m_active)
        deactivate();
    
    //Configure for GPIO pin number (num)
    if (num < 0 or num > MAX_GPIO)
    {
        m_sGPIONum = "";
        m_GPIONum = -1;
        return -1;
    }
    
    // Create the integer and string repesentations of the GPIO number
    m_GPIONum = num;
    std::ostringstream convert;
    convert << m_GPIONum;
    m_sGPIONum = convert.str();
    
    return 0;
}


/** @brief Create the GPIO SYSFS interface to access the attached GPIO pin.
 *
 *  Overridden to open the SYSFS value file for this pin and use the linux
 *  filesystem access method to operate
 *
 *	@return int: Result code.  Negative numbers represent failure.
 */
inline int FSGPIO_Pin::activate()
{
	int result = 0;
    
	// Do the base class activation code
    {
        std::string export_str = STR_EXPORT_FN;
        std::string dirfile_str = STR_GPIO_PRE + m_sGPIONum + STR_DIR_POST;
        
        // Check to see if the export directory has already been created.  If it has then quit with
        // a resource error code to indicate this.
        if (access(dirfile_str.c_str(), F_OK) != -1)
            return GPIO_RESERR;
        
        // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
        std::ofstream exportgpio(export_str.c_str());
        if (!exportgpio)
            return GPIO_FILEERR;
        exportgpio << m_sGPIONum ; 						// Write GPIO number to export
        exportgpio.close(); 							// Close export file
        
        m_dirFd = open(dirfile_str.c_str(), O_RDWR);	// Open the direction file for read/write
        if (m_dirFd < 0)
            return GPIO_FILEERR;
    }
    
    
	// Open the SYSFS value file for this pin.
    std::string valfile_str = STR_GPIO_PRE + m_sGPIONum + STR_VALUE_POST;
	m_valFd = open(valfile_str.c_str(), O_RDWR);
	if (m_valFd < 0)
		return GPIO_FILEERR;
    
    m_active = 1;
	return result;
}


/** @brief Create the GPIO SYSFS interface to access the requested GPIO pin.
 *
 *  Overridden to open the SYSFS value file for the given kernel GPIO number
 *
 *	@param num: representing the GPIO number to attach to.
 *	@return int: Result code.  Negative numbers represent failure.
 */
inline int FSGPIO_Pin::activate(int num)
{
	int result = 0;
    
	// Connect the GPIO number to this pin
	result = connectGPIO(num);
	if (result< 0)
		return result;
    
	// Activate the GPIO SYSFS interface
	result = activate();
	return result;
}


/** @brief Dismantle the GPIO SYSFS interface to the attached GPIO pin.
 *
 *	@return	int: Result code.  Negative numbers represent failure.
 */
inline int FSGPIO_Pin::deactivate()
{
	{
        std::string unexport_str = STR_UNEXPORT_FN;
        
        // If the pin is not setup then abort with error
        if (m_active < 1)
            return GPIO_RDYERR;
        
        // Open unexport file
        std::ofstream unexportgpio(unexport_str.c_str());
        if (unexportgpio < 0)
            return GPIO_FILEERR;
        
        // Write GPIO number to unexport
        unexportgpio << m_sGPIONum ;
        unexportgpio.close();
        
        // Close the direction file
        close(m_dirFd);
        m_dirFd = -1;
        m_active = 0;
        return 0;
    }
	close(m_valFd);
	m_valFd = -1;
	return 0;
}


/** @brief Set the value of the GPIO pin using the SYSFS value file
 *
 *	@param val: Digital value for the pin state [GPIO_HIGH|GPIO_LOW].
 */
inline void FSGPIO_Pin::set(int val)
{
	if (m_active < 1)
		return;
    
	if (val == GPIO_HIGH)
		write(m_valFd, "1\n", 2);
	else
		write(m_valFd, "0\n", 2);
}


/** @brief Read the value of the GPIO pin from the SYSFS value file
 *
 *	@return	int: Value of the pin [GPIO_HIGH|GPIO_LOW].
 */
inline int FSGPIO_Pin::get()
{
	char		c_val;
    
	// If the pin is not setup return error
	if (m_active < 1)
		return GPIO_RDYERR;
    
	// Rewind to the beginning of the file and read
	// a single character.
	lseek(m_valFd, 0L, SEEK_SET);
	read(m_valFd, &c_val, 1);
    
	// Determine the integer value for the character value
	if (c_val == '1')
		return GPIO_HIGH;
	else
		return GPIO_LOW;
}


/** @brief Set the GPIO pin's I/O direction.
 *
 *	@param dir: The desired input/output type 0=input, 1=output.
 *	@return int: Result code.  Negative numbers indicate failure
 */
inline int FSGPIO_Pin::set_dir(int dir)
{
    std::string	s_dir;
    
    // If the pin is not setup then abort with error
    if (m_active < 1)
        return GPIO_RDYERR;
    
    // Choose the correct string to write to the file
    lseek(m_dirFd, 0L, SEEK_SET);
    if (dir == GPIO_OUT)
    {
        s_dir = STR_OUT;
        m_dir = GPIO_OUT;
    }
    else
    {
        s_dir = STR_IN;
        m_dir = GPIO_IN;
    }
    
    // Write the direction value to the GPIO direction file
    write(m_dirFd, s_dir.c_str(), s_dir.size());
    
    return 0;
}


/** @brief Get the GPIO pin I/O direction value.
 *
 *  @return int: Direction value [GPIO_IN|GPIO_OUT]
 */
inline int FSGPIO_Pin::get_dir()
{
    return m_dir;
}

#endif // FSGPIO_PIN_H
