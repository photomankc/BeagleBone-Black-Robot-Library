#include <stdlib.h>
#include "tmp102.h"

/** @brief Create a TMP102 temp sensor object with captive I2C bus
 *
 *  Construct a TMP102 sensor object using an I2C bus object that will be owned
 *  by the sensor.  The bus object must remain valid for the life of the sensor
 *  object and will be destroyed by the sensor when it goes out of scope. The
 *  address defaults to the standard address for the TMP102 sensor chip.
 *
 *  @param bus: Pointer for the I2C bus object to use for communication.
 *  @param address: The address of the TMP102 sensor on the I2C bus.
 */
TMP102::TMP102(I_I2C* bus, uint8_t address)
{
    ownBus  = 0;
    p_bus   = 0;
    adr     = 0;
    oneShotActive = 0;
    oneShotTrigger = 0;
    enabled = 0;
    cfgCache = -1;
    
    if (bus)
    {
        p_bus = bus;
        ownBus = 1;
        adr = address;
        setEnable(1);
    }
}


/** @brief Create a TMP102 temp sensor object with shared I2C bus
 *
 *  Construct a TMP102 sensor object using an I2C bus object that will not be 
 *  owned by the sensor.  The bus object must remain valid for the life of the 
 *  sensor object.  The address defaults to the standard address for the TMP102 
 *  sensor chip.
 *
 *  @param bus: Reference to the I2C bus object to use for communication.
 *  @param address: The address of the TMP102 sensor on the I2C bus.
 */
TMP102::TMP102(I_I2C& bus, uint8_t address)
{
    ownBus  = 0;
    oneShotActive = 0;
    oneShotTrigger = 0;
    cfgCache = -1;
    p_bus = &bus;
    adr = address;
    setEnable(1);
}


/** @brief Destroys the object and cleans-up the bus object if it is owned.
 */
TMP102::~TMP102()
{
    if (ownBus)
        delete p_bus;
}


/** @brief Indicates if the device is online and ready to use.
 *
 *  @return int: 1 if ready or 0 otherwise
 */
int TMP102::isReady()
{
    // Read the config register and if there is a valid value
    // then we seem to have established communication.
    int32_t cfgVal = read(TMP102_REG_CFG);
    if ((cfgVal >= 0xFFFF) | (cfgVal < 0))
        return 0;
    else
        return 1;
}


/** @brief Returns the currently measured temperature in degrees F.
 *
 *  @return float: Measured temperature in degrees F.  < -1024 for error.
 */
float TMP102::getTemp_F()
{
    if(enabled)
    {
        float tempC = getTemp_C();
        if (tempC <= TMP102_ERR_BUS)
            return tempC;
        return (tempC * 1.8) + 32;
    }
    else
        return TMP102_ERR_TEMP;
}


/** @brief Returns the currently measured temperature in degrees C.
 *
 *  This is the base temperature measurement function.  If called while device
 *  is in continuous conversion mode the function will return the temperature
 *  in C immeadiately after it is read.  If the device is in one-shot mode the
 *  function will return TMP102_ERR_NRDY until the conversion is complete or the
 *  mode is changed.  This prevents the function from blocking while waiting 
 *  for a conversion.
 *
 *  @return float: Measured temperature in degrees C. < -1024 for error.
 */
float TMP102::getTemp_C()
{
    if (enabled)
    {
        if (oneShotActive)
        {   // Oneshot mode logic.  Triggers conversion if needed.
            if (!oneShotTrigger)
            {
                triggerOneShot();
                return TMP102_ERR_NRDY;
            }
            
            int osRdy = oneShotReady();
            if (!osRdy)
            {
                return TMP102_ERR_NRDY;
            }
            else if (osRdy)
            {
                oneShotTrigger = 0;
            }
        }
        
        // Obtain the temp reg value and convert to C.
        int temperatureSum = read(TMP102_REG_TEMP);
        if (temperatureSum <= TMP102_ERR_BUS)
            return temperatureSum;  // Return error value
        
        temperatureSum >>= 4;
        return temperatureSum * 0.0625;
    }
    else
        return TMP102_ERR_TEMP;
}


/** @brief Enables or disables the object and device.
 *  
 *  @param val: 0 for disable, otherwise enable.
 *  @return int: New enable state.
 */
int TMP102::setEnable(int val)
{
    if (val)
    {
        enabled = 1;
        setOneShot(0);  // Clears shutdown in CFG register
    }
    else
    {
        setOneShot(1);  // Sets shutdown in CFG register
        enabled = 0;
    }
    
    return enabled;
}


/** @brief Returns the value stored in the chip configuration register.
 *
 *  @return int32_t: The 16 bit config register value or -1 if there is a fault.
 */
int32_t TMP102::getConfig(int force)
{
    
    if (enabled)
    {
        if (force == TMP102_CACHE_FORCE)
        {   // Perform a forced read of the config register.
            cfgCache = read(TMP102_REG_CFG);
            return cfgCache;
        }
        else
        {   // Use cached version of register if we have one. Or read the
            // register if we don't.
            if (cfgCache >= 0)
            {
                return cfgCache;
            }
            else
            {
                cfgCache = read(TMP102_REG_CFG);
                return cfgCache;
            }
        }
            
    }
    else
        return -1;
}


/** @brief Set the configuration register to the given value.
 *
 *  @param cfg: The value to store in the rconfig egister.
 */
void TMP102::setConfig(uint16_t cfg)
{
    if (enabled)
    {
        write(TMP102_REG_CFG, cfg);
        cfgCache = -1;  //Clear the cached value.
    }
}


/** @brief Enable or disable the one-shot mode of operation.
 *
 *  @param val: Setting is 1 for enabled or 0 for disabled.
 */
void TMP102::setOneShot(uint16_t val)
{
    if (enabled)
    {
        // One shot is availible when the device is in shutdown mode
        int32_t regVal = getConfig();
        if (regVal >= 0)
        {
            if (val)
            {
                regVal |= TMP102_MASK_CFG_SD;
                oneShotActive = 1;
                oneShotTrigger = 0;
            }
            else
            {
                regVal &= ~TMP102_MASK_CFG_SD;
                oneShotActive = 0;
                oneShotTrigger = 0;
            }
            
            setConfig(regVal);
        }
    }
}


/** @brief Returns the one-shot mode setting.
 *
 *  @return int: 1 if enabled or 0 otherwise.  -1 for error.
 */
int TMP102::getOneShot()
{
    if (enabled)
    {
        // One shot is available if the device is in shutdown mode
        int32_t regVal = getConfig();
        if(regVal >= 0)
        {
            int32_t shutDwn = regVal & TMP102_MASK_CFG_SD;
            
            if(shutDwn)
                return 1;
            else
                return 0;
        }
        else
            return -1;
    }
    else
        return -1;
}


/** @brief Returns the conversion rate config value.
 *
 *  @return int: 0-3 representing the four possible settings.  -1 for error.
 */
int TMP102::getConversionRate()
{
    if (enabled)
    {
        int32_t regVal = getConfig();
        if (regVal >= 0)
        {
            int rate = regVal & TMP102_MASK_CFG_CR;
            
            return rate >> 6;
        }
        else
            return -1;
    }
    else
        return -1;
}


/** @brief Set the conversion rate config value
 *
 *  @param val: 0-3 representing the 4 possible settings.
 */
void TMP102::setConversionRate(uint16_t val)
{
    if (enabled)
    {
        int32_t regVal = getConfig();
        if (regVal >= 0)
        {
            if (val > 3) val = 3;
            val <<= 6;
            
            regVal &= ~TMP102_MASK_CFG_CR;
            regVal |= val;
            setConfig(regVal);
        }
    }
}


/* 
 * Read the value from the specified register.  Returns the device register
 * value or -1 if bus failure.
 */
int32_t TMP102::read(int16_t reg)
{
    int err = 0;

    uint32_t val = 0;
    if (p_bus)
    {
        err = p_bus->openBus(adr);
        if (err == 0)
        {
            val = p_bus->rxWord(reg);
            p_bus->closeBus();
        }
        else
            return err;
    }
    else
        return -1;
    
    // Have to reverse the byte order.  TMP102 transmits in reverse to expected
    // word byte order.
    uint8_t bytes[2];
    bytes[TMP102_MSB] = val;
    bytes[TMP102_LSB] = val >> 8;
    val = (bytes[TMP102_MSB] << 8) | bytes[TMP102_LSB];
    return val;
}


/*
 * Write a value to the specified register on the chip.  reg is the register 
 * address 0-3.  val is the value to store in the register.  -1 indicates 
 * failure.
 */
int TMP102::write(int16_t reg, uint16_t val)
{
    // Reverse the byte order to transmit word as the TMP102 expects.
    uint8_t bytes[2];
    bytes[TMP102_LSB] = val;
    bytes[TMP102_MSB] = val >> 8;
    val = (bytes[TMP102_LSB] << 8) | bytes[TMP102_MSB];
    
    int err = 0;
    if (p_bus)
    {
        err = p_bus->openBus(adr);
        if (err == 0)
        {
            err = p_bus->txWord(reg, val);
            p_bus->closeBus();
            if (err < 0)
                return -1;
        }
        else
            return -1;
    }
    else
        return -1;
    
    return err;
}


/*
 *  Writes the config register bit needed to start a one-shot conversion.  All
 *  other config register bits are uneffected.
 */
void TMP102::triggerOneShot()
{
    if (enabled)
    {
        // Trigger a one-shot conversion by writing 1 to OS bit.
        int32_t regVal = getConfig(TMP102_CACHE_FORCE);
        if(regVal >= 0)
        {
            regVal |= TMP102_MASK_CFG_OS;
            oneShotTrigger = 1;
            setConfig(regVal);
        }
    }
}


/*
 *  Checks to determine if a one-shot conversion has completed.  Returns 1 if
 *  one-shot bit is clear. Returns 0 otherwise.
 */
int TMP102::oneShotReady()
{
    if (enabled)
    {   // Do a forced config read since the bit can change without a write.
        int32_t regVal = getConfig(TMP102_CACHE_FORCE);
        if (regVal >= 0)
        {
            if (regVal & TMP102_MASK_CFG_OS)
                return 1;
            else
                return 0;
        }
        else
            return 0;
    }
    else
        return 0;
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