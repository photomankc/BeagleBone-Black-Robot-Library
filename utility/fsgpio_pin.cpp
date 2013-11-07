#include "fsgpio_pin.h"
using namespace std;


/** @brief Default constructor.  
 * 
 *  Creates an unattached GPIO object
 */
FSGPIO_Pin::FSGPIO_Pin()
{
	m_GPIONum = -1;
	m_sGPIONum = "";
	m_valFd = -1;
	m_dirFd = -1;
	m_active = 0;
}


/** @brief Constructor.  
 *
 *  Creates an object attached to the specified kernel GPIO pin number
 *
 *	@param num: integer The GPIO number to attach to.
 */
FSGPIO_Pin::FSGPIO_Pin(int num)
{
	m_GPIONum = -1;
	m_sGPIONum = "";
	m_active = 0;
	m_valFd = -1;
	m_dirFd = -1;

	// Attach this pin to the requested GPIO number
	GPIO_Pin::connectGPIO(num);
}


/** @brief Destructor
 *
 */
FSGPIO_Pin::~FSGPIO_Pin()
{
	FSGPIO_Pin::deactivate();
}



/** @brief Create the GPIO SYSFS interface to access the attached GPIO pin.  
*
*   Overridden to open the SYSFS value file for this pin.
*
*	@return Integer result code.  Negative numbers represent failure.
*/
int FSGPIO_Pin::activate()
{
	int result = 0;

	// Do the base class activation code
	result = GPIO_Pin::activate();
	if (result < 0)
		return result;

	// Open the SYSFS value file for this pin.
	string valfile_str = STR_GPIO_PRE + m_sGPIONum + STR_VALUE_POST;
	m_valFd = open(valfile_str.c_str(), O_RDWR);
	if (m_valFd < 0)
		return GPIO_FILEERR;

	return result;
}


/** @brief Create the GPIO SYSFS interface to access the requested GPIO pin.  
 *
 *  Overridden to open the SYSFS value file for the given kernel GPIO number
 *
 *	@param Integer value representing the GPIO number to attach to.
 *	@return Integer result code.  Negative numbers represent failure.
 */
int FSGPIO_Pin::activate(int num)
{
	int result = 0;

	// Connect the GPIO number to this pin
	result = GPIO_Pin::connectGPIO(num);
	if (result< 0)
		return result;

	// Activate the GPIO SYSFS interface
	result = activate();
	return result;
}


/** @brief Dismantle the GPIO SYSFS interface to the attached GPIO pin.  
 *
 *  Overridden to close the SYSFS value file as well.
 *
 *	@return	Int: Result code.  Negative numbers represent failure.
 */
int FSGPIO_Pin::deactivate()
{
	GPIO_Pin::deactivate();
	close(m_valFd);
	m_valFd = -1;
	return 0;
}


/** @brief Set the value of the GPIO pin using the SYSFS value file
 *
 *	@param val Integer value for the pin state 0|1
 */
void FSGPIO_Pin::set(int val)
{
	if (m_active < 1)
		return;

	if (val == HIGH)
		write(m_valFd, "1\n", 2);
	else
		write(m_valFd, "0\n", 2);
}


/** @brief Read the value of the GPIO pin from the SYSFS value file
 *
 *	@return	Integer value of the pin [0|1]
 */
int FSGPIO_Pin::get()
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
		return HIGH;
	else
		return LOW;
}

int FSGPIO_Pin::get_GPIONum()
{ return m_GPIONum; }
