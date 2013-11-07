#include "gpio_pin.h"

using namespace std;

/** Connect the GPIO object to the specified GPIO pin number.
*
*	\param num Integer GPIO number to connect this object to.
*	\return Integer result code.  Negative numbers represent failure
*/
int GPIO_Pin::connectGPIO(int num)
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
}


/** Create the GPIO SYSFS interface to access the attached GPIO pin
*
*	\return Integer result code.  Negative numbers represent failure.
*/
int GPIO_Pin::activate()
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
}


/** Create the GPIO SYSFS interface to access the requested GPIO pin
*
*	\param Integer value representing the GPIO number to attach to.
*	\return Integer result code.  Negative numbers represent failure.
*/
int GPIO_Pin::activate(int num)
{
	// Connect the requested GPIO number to this pin
	int result;
	result = connectGPIO(num);
	if (result< 0)
		return result;

	// Activate the SYSFS interface for this GPIO pin
	result = activate();
	return result;
}


/** Dismantle the GPIO SYSFS interface to the attached GPIO pin
*
*	\return	Integer result code.  Negative numbers represent failure.
*/
int GPIO_Pin::deactivate()
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
}


/** Set the GPIO pin's I/O direction.
*
*	\param dir Integer representing the desired input/output type 0=input, 1=output.
*	\return Integer result code.  Negative numbers indicate failure
*/
int GPIO_Pin::set_dir(int dir)
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
}



int GPIO_Pin::get_GPIONum()
{ return m_GPIONum; }
