#ifndef GPIO_PIN_H
#define GPIO_PIN_H

/** GPIO_Pin Class.
* Class to represent BeagleBone GPIO pins.  Base class to represent basic interface to GPIO pins
* Child classes define the specfic interfaces to different methods of GPIO access.  All GPIO classes
* will implement the basic set of funtions here.
*
*   \author     Kyle Crane
*   \version    0.9.0
*/

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

using namespace std;

enum GPIO_CONSTANTS
{
    MAX_GPIO = 125,
    HIGH     = 1,
    LOW      = 0,
    INPUT    = 0,
    OUTPUT   = 1
};

enum GPIO_ERRORS
{
    GPIO_GENERR  = -1,
    GPIO_FILEERR = -2,
    GPIO_RDYERR  = -3,
    GPIO_RESERR  = -4,
};


class GPIO_Pin
{
 protected:
    string m_sGPIONum;  /**< String representing the GPIO pin number this oject controls (used for file operations) */
    int    m_GPIONum;   /**< Integer representing the GPIO pin number this object controls */
    int    m_active;    /**< Flag indicating that the pin is exported without error. */
    int    m_dirFd;     /**< File handle for the pin value file; */

public:
    virtual ~GPIO_Pin() {};
    virtual int connectGPIO(int num);   // connect object to a GPIO pin.
    virtual int activate();             // Activate the pin by exporting the attached GPIO number
    virtual int activate(int num);      // Attach to the specified GPIO number and activate the pin
    virtual int deactivate();           // unexport GPIO
    virtual int set_dir(int dir);       // Set GPIO Direction
    virtual int get_dir() {return 0;};  // Get the current GPIO Direction
    virtual void set(int val)=0;        // Set GPIO Value (putput pins)
    virtual int get()=0;                // Get GPIO Value (input/ output pins)
    virtual int get_GPIONum();          // Return the GPIO number associated with the instance of an object
};

#endif // GPIO_PIN_H
