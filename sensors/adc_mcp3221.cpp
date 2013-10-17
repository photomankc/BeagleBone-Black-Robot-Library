#include <unistd.h>
#include "adc_mcp3221.h"

/** @brief Default Constructor
  *
  * Setup a disconnected ADC object
  */
ADC_MCP3221::ADC_MCP3221()
{
    m_bus			= 0;
    m_adr			= 0;
    m_countAvg      = 0;
    m_cal			= 0;
    m_vref			= 0;
    m_vscale		= 1;
}


/** @brief Constructor
  *
  * Setup the ADC object and assign it an I2C bus reference
  * to use for communication.  Device address is default
  *
  * @param p_bus Pointer to I_I2C bus that the ADC is connected to
  * @param adr 7bit address of device if not default.
  */
ADC_MCP3221::ADC_MCP3221(I_I2C* p_bus, uint8_t adr)
{ init(p_bus, adr); }

ADC_MCP3221::ADC_MCP3221(I_I2C* p_bus, int cal, float vref)
{ init(p_bus, cal, vref); }

ADC_MCP3221::ADC_MCP3221(I_I2C* p_bus, int cal, float vref, float vscale)
{ init(p_bus, cal, vref, vscale); }


/** @brief Default Destructor
  *
  *
  */
ADC_MCP3221::~ADC_MCP3221()
{
    if(m_bus) delete m_bus;
    m_adr       = 0;
    m_countAvg  = -1;
}


/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param p_bus I_I2C Bus object pointer to be used to communicate.
  * @param adr I2C address for the ADC device
  */
void ADC_MCP3221::init(I_I2C* p_bus, uint8_t adr)
{
    m_adr = adr;
	init(p_bus, 0, 1, 1);
}


/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param p_bus I_I2C Bus object pointer to be used to communicate.
  * @param cal Integer value for calibrating the count to match actual measured voltage
  * @param vref Float value that represents the voltage represented by a full-scale count
  */
void ADC_MCP3221::init(I_I2C* p_bus, int cal, float vref)
{
    m_adr = MCP3221_ADR_DFLT;
	init(p_bus, cal, vref, 1);
}

/** @brief init
  *
  * Setup the ADC object and assign it an I2C bus object
  * to use for communication.
  *
  * @param bus I2C Bus object pointer to be used to communicate.
  * @param adr Value for the  7bit I2C address of the ADC device
  * @param cal  Integer value for calibrating the count to match actual measured voltage
  * @param vref	Value that represents the voltage represented by a full-scale count
  * @param vscale Value that represents the correction needed to obtain the full input voltage if it is has been scaled up or down.
  */
void ADC_MCP3221::init(I_I2C* p_bus, int cal, float vref, float vscale)
{
	m_cal       = 0;
	m_vref      = 0.0;
	m_vscale    = 0.0;
	m_countAvg  = -1;

	if(p_bus) m_bus = p_bus;
    m_adr = MCP3221_ADR_DFLT;
	set_cal(cal);
	set_vref(vref);
	set_vscale(vscale);
}

/** @brief setCal
 *
 *  Set calibration value for the ADC reading.  This value is added to the raw
 *  count to correct the reading until it matches expected or measured result.  
 *  Value is applied to the final average result.
 *
 *  @param val  - Integer value for calibration [-512 ... 512]
 */
void ADC_MCP3221::set_cal(int val)
{
    if (val > 512)
        val = 512;
    else if (val < -512)
        val = -512;

    m_cal = val;
}


/** @brief Set the reference voltage value for the ADC.
 *
 *  Set the reference voltage value for the ADC.  This is the maximum voltage
 *  represented by a full scale count vale.
 *
 * @param val Represents the reference voltage the ADC is using
 */
void ADC_MCP3221::set_vref(float val)
{
	if (val < 1)
		val *= -1.0;

	m_vref = val;
}


/** @brief Set the voltage scale value.
 *
 *  This represents the correction factor required to go from the ADC voltage 
 *  reading to the real world maximum voltage being sampled.  This is used
 *  when a voltage divider is being used to reduce a larger voltage to an 
 *  acceptable range for the ADC.
 *
 *  @param val The value to scale the voltage reading by.
 *
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
  * @param  sCnt [1 - 100] Specifies the number of samples to collect and average.
  * @return Int: The averaged count value computed from the samples.
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
        if (m_bus->openBus(m_adr) == 0)
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

    // Return error if the results are not in range
    if (result > MCP3221_MAX_COUNT)
	{
		 m_countAvg = 0;
        return ERR_I2C_RNG;
	}

    m_countAvg = result + m_cal;
    return result;
}


/** @brief Return the voltage represented by the current ADC count
 *
 *  Returns the decimal voltage represented by the ADC count value using the
 *  objects VREF and VSCALE values.
 *
 *  @return	float: The current voltage measured based on the digital count
 */
float ADC_MCP3221::getVolts()
{
    if (m_vref == 0)
        return -1;

    float   vpc = m_vref / 4096.0;
    float   vIn = vpc * m_countAvg;	// Calculate the input voltage on the ADC
    return (vIn/m_vscale);			// Calculate the scaled voltage represented.
}


/** @brief Return the voltage represented by the current ADC count
 *
 *  Returns the decimal voltage represented by the ADC count value given the
 *  supplied reference voltage value and scale.
 *
 *  @param  vRef Value that represents the alternate ADC reference voltage for a full-scale count
 *  @param  vScale	Value represents the alternate correction needed to calculate the real input voltage
 *  @return	float: The current voltage measurement based on the digital count
 */
float ADC_MCP3221::getVolts(float vRef, float vScale)
{
    if (vRef == 0)
        return -1;
	if (vScale <= 0)
		return -1;

    float   vpc = vRef / 4096.0;
    float   vIn = vpc * m_countAvg;	// Calculate the input voltage on the ADC
    return (vIn/vScale);			// Calculate the scaled voltage represented.
}


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

