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
    FSGPIO_Pin();               // Create a GPIO pin object
    FSGPIO_Pin(int num);        // Create a GPIO pin object controlling given pin
    virtual ~FSGPIO_Pin();      // Destroy GPIO pin object

    int     activate();         // Exports GPIO.
    int     activate(int num);  // Attach and Export the GPIO number resquested.
    int     deactivate();       // Unexport GPIO.
    void    set(int val);       // Set GPIO Value (putput pins)
    int     get();              // Get GPIO Value (input/ output pins)
    int     get_GPIONum();      // Return the kernel GPIO number
    
private:
    int		m_valFd;            // File handle for GPIO pin.
};

#endif // FSGPIO_PIN_H
