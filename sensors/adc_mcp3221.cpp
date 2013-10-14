////////////////////////////////////////////////////////////////////////////////////////////////
//
//  adc_mcp3221.cpp
//
//  Implementation file for ADC_MCP3221 I2C Analog to Digital converter.
//
//

#include <unistd.h>
#include "ADC_MCP3221.h"

/** @brief Default Constructor
  *
  * Setup a disconnected ADC object
  */
ADC_MCP3221::ADC_MCP3221()
{
    m_bus			= 0;
    m_addr			= 0;
    m_countAvg	= 0;
    m_cal				= 0;
    m_vref			= 0;
    m_vscale		= 1;
}


/** @brief Constructor
  *
  * Setup the ADC object and assign it an I2C bus reference
  * to use for communication.  Device address is default
  *
  * @param bus - Reference to I2C bus that the ADC is connected to
  */
ADC_MCP3221::ADC_MCP3221(I2C& bus)
{ init(bus); }


/** @brief Constructor
  *
  * Setup the ADC object and assign it an I2C bus reference
  * to use for communication.
  *
  * @param bus  - Reference to I2C bus that the ADC is connected to.
  * @param addr - The 7bit address of the ADC device.
  */
ADC_MCP3221::ADC_MCP3221(I2C& bus, uint8_t addr)
{ init(bus, addr); }

ADC_MCP3221::ADC_MCP3221(I2C& bus, uint8_t addr, int cal, float vref)
{ init(bus, addr, cal, vref); }

ADC_MCP3221::ADC_MCP3221(I2C& bus, uint8_t addr, int cal, float vref, float vscale)
{ init(bus, addr, cal, vref, vscale); }


/** @brief Default Destructor
  *
  *
  */
ADC_MCP3221::~ADC_MCP3221()
{
    m_bus       = 0;
    m_addr      = 0;
    m_countAvg  = -1;
}


/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param bus - I2C Bus object reference to be used to communicate.
  */
void ADC_MCP3221::init(I2C& bus)
{
    init(bus, MCP3221_ADR_DFLT, 0, 1, 1);
}


/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param bus  - I2C Bus object reference to be used to communicate.
  * @param addr - I2C address for the ADC device
  */
void ADC_MCP3221::init(I2C& bus, uint8_t addr)
{
	init(bus, addr, 0, 1, 1);
}


/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param bus  	- I2C Bus object reference to be used to communicate.
  * @param addr 	- uint8_t value for the  7bit I2C address of the ADC device
  * @param cal		- Integer value for calibrating the count to match actual measured voltage
  * @param vref		- float value that represents the voltage represented by a full-scale count
  */
void ADC_MCP3221::init(I2C& bus, uint8_t addr, int cal, float vref)
{
	init(bus, addr, cal, vref, 1);
}

/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param bus  	- I2C Bus object reference to be used to communicate.
  * @param addr 	- uint8_t value for the  7bit I2C address of the ADC device
  * @param cal		- Integer value for calibrating the count to match actual measured voltage
  * @param vref		- float value that represents the voltage represented by a full-scale count
  * @param vscale	- float value that represents the correction needed to obtain the full input voltage if it is has been scaled up or down.
  */
void ADC_MCP3221::init(I2C& bus, uint8_t addr, int cal, float vref, float vscale)
{
	m_cal       		= 0;
	m_vref			= 0.0;
	m_vscale			= 0.0;
	m_countAvg  = -1;

	m_bus = &bus;
	m_addr = addr;
	set_cal(cal);
	set_vref(vref);
	set_vscale(vscale);
}

/** @brief setCal
  *
  * Set calibration value for the ADC reading.  This value is added to the raw count
  * to correct the reading until it matches expected or measured result.  Value is
  * applied to the final average result.
  *
  * @param val  - Integer value for calibration [-512 ... 512]
  */
void ADC_MCP3221::set_cal(int val)
{
    if (val > 512)
        val = 512;
    else if (val < -512)
        val = -512;

    m_cal = val;
}


/** @brief set_vref
  *
  * Set the reference voltage value for the ADC.  This is the maximum voltage represented by a
  * full scale count vale.
  *
  * @param val  - Floating point number that represents the reference voltage the ADC is using
  */
void ADC_MCP3221::set_vref(float val)
{
	if (val < 1)
		val *= -1.0;

	m_vref = val;
}


/** @brief set_vref
  *
  * Set the reference voltage value for the ADC.  This is the maximum voltage represented by a
  * full scale count vale.
  *
  * @param val  - Floating point number that represents the correction needed to change the ADC input
  *							voltage to the actual input voltage.  The ADC voltage measurement is divided by this
  *							number to compute the actual input voltage.  Zero is illegal.
  */
void ADC_MCP3221::set_vscale(float val)
{
	if (val == 0)
		val=1.0;

	if (val < 0)
		val *= -1.0;

	m_vscale = val;
}

/** @brief update
  *
  * Updates the internal count average with current readings from the ADC.  Reading is
  * sampled the number of times specified and a simple running average is performed.
  *
  * @param  sCnt    - Integer, [1 ... 100], that specifies the number of samples to collect and average.
  * @return         - Integer, the averaged count value computed from the samples.
  */
int ADC_MCP3221::update(int sCnt)
{
    if (sCnt < 1)
        sCnt = 1;

    if (sCnt > 100)
        sCnt = 100;

    int result = -1;
    if(m_bus)
    {
        if (m_bus->openBus(m_addr) == 0)
        {
            result = 0;
            for(int i=sCnt; i>0; i--)
                result += m_bus->rxWord();

            m_bus->closeBus();
        }
        else
		{
			m_countAvg = 0;
			return ERR_I2C_GEN;
		}

        result /= sCnt;
    }

    /* Return error if the results are not in range */
    if (result > MCP3221_MAX_COUNT)
	{
		 m_countAvg = 0;
        return ERR_I2C_RNG;
	}

    m_countAvg = result + m_cal;
    return result;
}


/** @brief getVolts - Return the voltage represented by the current ADC count
  *
  * Returns the decimal voltage represented by the ADC count value using the objects VREF and VSCALE
  * values.
  *
  * @return				- float value representing the current voltage measured based on the digital count
  */
float ADC_MCP3221::getVolts()
{
    if (m_vref == 0)
        return -1;

    float   vpc = m_vref / 4096.0;
    float   vIn = vpc * m_countAvg;	// Calculate the input voltage on the ADC
    return (vIn/m_vscale);					// Calculate the scaled voltage represented.
}


/** @brief getVolts - Return the voltage represented by the current ADC count
  *
  * Returns the decimal voltage represented by the ADC count value given the supplied reference voltage
  * value and scale.
  *
  * @param  vRef		- Floating point number that represents the alternate ADC reference voltage for a full-scale count
  * @param  vScale	- Floating point number that represents the alternate correction needed to calculate the real input voltage
  * @return					- Floating point number representing the current voltage measured based on the digital count
  */
float ADC_MCP3221::getVolts(float vRef, float vScale)
{
    if (vRef == 0)
        return -1;
	if (vScale <= 0)
		return -1;

    float   vpc = vRef / 4096.0;
    float   vIn = vpc * m_countAvg;	// Calculate the input voltage on the ADC
    return (vIn/vScale);						// Calculate the scaled voltage represented.
}

