#ifndef GPIO_PIN_H
#define GPIO_PIN_H

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define     STR_EXPORT_FN    "/sys/class/gpio/export"
#define     STR_UNEXPORT_FN  "/sys/class/gpio/unexport"
#define     STR_GPIO_PRE     "/sys/class/gpio/gpio"
#define     STR_VALUE_POST   "/value"
#define     STR_DIR_POST     "/direction"
#define     STR_EDGE_POST    "/edge"
#define     STR_ACTLOW_POST  "/active-low"
#define     STR_OUT          "out"
#define     STR_IN           "in"
#define     STR_HIGH         "1"
#define     STR_LOW          "0"

enum GPIO_CONSTANTS
{
    MAX_GPIO    = 125,
    GPIO_HIGH   = 1,
    GPIO_LOW    = 0,
    GPIO_IN     = 0,
    GPIO_OUT    = 1
};


enum GPIO_ERRORS
{
    GPIO_OK      = 0,
    GPIO_GENERR  = -1,
    GPIO_FILEERR = -2,
    GPIO_RDYERR  = -3,
    GPIO_RESERR  = -4,
};

/** @brief Generic GPIO pin class allowing basic digital I/O functions.
 *  
 *  This represents a base class for all GPIO_Pin types.  Custom implentations
 *  should override this class and provide the interface functions spelled out
 *  here.  Hardware specific information should managed in the gpio_pin_defs.h 
 *  file using defines to implement for specific hardware platforms as GPIO 
 *  availiblity varies wildy between them.
 *
 *  @author     Kyle Crane
 *  @version    1.0.0
 */
class GPIO_Pin
{
 protected:
    std::string m_sGPIONum;  /**< String representing the GPIO pin number  */
    int         m_GPIONum;   /**< Integer representing the GPIO pin number */
    int         m_active;    /**< Flag indicating activated without error. */
    int         m_dir;       /**< Pin direction */
    
public:
    virtual ~GPIO_Pin() {};
    
    /** @brief Connect this object to the requested pin number.
     *  @param num: Pin number (See gpio_pin_defs.h).
     *  @return int: Result code
     */
    virtual int connectGPIO(int num)=0;
    
    /** @brief Activate this object on the currently assigned pin.
     *  Initializes the pin with current configuration.
     *  @return int: Result code.
     */
    virtual int activate()=0;
    
    /** @brief Activate this object on the specified pin number.
     *  @param pin: Pin number (See gpio_pin_defs.h).
     *  @return int: Result code.
     */
    virtual int activate(int num)=0;
    
    /** @brief Deactivate this object and release pin.
     *  @return int: Result code.
     */
    virtual int deactivate()=0;
    
    /** @brief Set the pin direction for input or output
     *  @param dir: Direction is GPIO_IN or GPIO_OUT.
     *  @return int: Result code.
     */
    virtual int set_dir(int dir)=0;
    
    /** @brief Get the pin direction for this GPIO pin.
     *  @return int: GPIO_IN or GPIO_OUT.
     */
    virtual int get_dir()=0;
    
    /** @brief Set the GPIO value for this pin.
     *  @param val: Digital value to set.  Either GPIO_HIGH or GPIO_LOW
     */
    virtual void set(int val)=0;
    
    /** @brief Get the current digital value on this GPIO pin.
     *  @return int: Digital value of pin.  Either GPIO_HIGH or GPIO_LOW
     */
    virtual int get()=0;
    
    /** @brief Get the GPIO pin number
     *  @return int: Pin number.
     */
    virtual int get_GPIONum() {return m_GPIONum;};
};





/*int GPIO_Pin::connectGPIO(int num)
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
 ostringstream convert;
 convert << m_GPIONum;
 m_sGPIONum = convert.str();
 
 return 0;
 }*/


/*int GPIO_Pin::activate()
 {
 string export_str = STR_EXPORT_FN;
 string dirfile_str = STR_GPIO_PRE + m_sGPIONum + STR_DIR_POST;
 
 // Check to see if the export directory has already been created.  If it has then quit with
 // a resource error code to indicate this.
 if (access(dirfile_str.c_str(), F_OK) != -1)
 return GPIO_RESERR;
 
 // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
 ofstream exportgpio(export_str.c_str());
 if (!exportgpio)
 return GPIO_FILEERR;
 exportgpio << m_sGPIONum ; 						// Write GPIO number to export
 exportgpio.close(); 											// Close export file
 
 m_dirFd = open(dirfile_str.c_str(), O_RDWR);	// Open the direction file for read/write
 if (m_dirFd < 0)
 return GPIO_FILEERR;
 
 m_active = 1;
 return 0;
 }*/


/*int GPIO_Pin::activate(int num)
 {
 // Connect the requested GPIO number to this pin
 int result;
 result = connectGPIO(num);
 if (result< 0)
 return result;
 
 // Activate the SYSFS interface for this GPIO pin
 result = activate();
 return result;
 }*/



/*int GPIO_Pin::deactivate()
 {
 string unexport_str = STR_UNEXPORT_FN;
 
 // If the pin is not setup then abort with error
 if (m_active < 1)
 return GPIO_RDYERR;
 
 // Open unexport file
 ofstream unexportgpio(unexport_str.c_str());
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
 }*/


/*int GPIO_Pin::set_dir(int dir)
 {
 string	s_dir;
 
 // If the pin is not setup then abort with error
 if (m_active < 1)
 return GPIO_RDYERR;
 
 // Choose the correct string to write to the file
 lseek(m_dirFd, 0L, SEEK_SET);
 if (dir == 1)
 s_dir = STR_OUT;
 else
 s_dir = STR_IN;
 
 // Write the direction value to the GPIO direction file
 write(m_dirFd, s_dir.c_str(), s_dir.size());
 
 return 0;
 }*/


/*
 Copyright (C) 2013 Kyle Crane
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */


#endif // GPIO_PIN_H
