#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    
    if (bus)
    {
        p_bus = bus;
        ownBus = 1;
        adr = address;
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
    p_bus = &bus;
    adr = address;
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
 *  @return float: Measured temperature in degrees F.
 */
float TMP102::getTemp_F()
{
    float tempC = getTemp_C();
    if (tempC < 0)
        return -65535.0;
    return (tempC * 1.8) + 32;
}


/** @brief Returns the currently measured temperature in degrees C.
 *
 *  @return float: Measured temperature in degrees C.
 */
float TMP102::getTemp_C()
{
    if (oneShotActive)
    {
        triggerOneShot();
        while (!oneShotReady());
    }
    
    int temperatureSum = read(TMP102_REG_TEMP);
    
    if (temperatureSum < 0)
        return -65535.0;
    temperatureSum >>= 4;
    return temperatureSum * 0.0625;
}


/** @brief Returns the value stored in the chip configuration register.
 *
 *  @return int32_t: The 16 bit config register value or -1 if there is a fault.
 */
int32_t TMP102::getConfig()
{
    return read(TMP102_REG_CFG);
}


/** @brief Set the configuration register to the given value.
 *
 *  @param cfg: The value to store in the rconfig egister.
 */
void TMP102::setConfig(uint16_t cfg)
{
    write(TMP102_REG_CFG, cfg);
}


/** @brief Enable or disable the one-shot mode of operation.
 *
 *  @param val: Setting is 1 for enabled or 0 for disabled.
 */
void TMP102::setOneShot(uint16_t val)
{
    // One shot is availible when the device is in shutdown mode
    int32_t regVal = getConfig();
    
    if (val)
    {
        regVal |= TMP102_MASK_CFG_SD;
        oneShotActive = 1;
    }
    else
    {
        regVal &= ~TMP102_MASK_CFG_SD;
        oneShotActive = 0;
    }
    
    setConfig(regVal);
}


/** @brief Returns the one-shot mode setting.
 *
 *  @return int: 1 if enabled or 0 otherwise.
 */
int TMP102::getOneShot()
{
    // One shot is available if the device is in shutdown mode
    int32_t regVal = getConfig();
    int32_t shutDwn = regVal & TMP102_MASK_CFG_SD;
    
    if(shutDwn)
        return 1;
    
    return 0;
}


/** @brief Returns the conversion rate config value.
 *
 *  @return int: 0-3 representing the four possible settings.  -1 for error.
 */
int TMP102::getConversionRate()
{
    int32_t regVal = getConfig();
    int rate = regVal & TMP102_MASK_CFG_CR;
    
    return rate >> 6;
}


/** @brief Set the conversion rate config value
 *
 *  @param val: 0-3 representing the 4 possible settings.
 */
void TMP102::setConversionRate(uint16_t val)
{
    int32_t regVal = getConfig();
    
    if (val > 3) val = 3;
    val <<= 6;
    
    regVal &= ~TMP102_MASK_CFG_CR;
    regVal |= val;
    setConfig(regVal);
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
    
    // Have to reverse the byte order.
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
    // Reverse the byte order to transmit porperly.
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
    // Trigger a one-shot conversion by writing 1 to OS bit.
    int32_t regVal = getConfig();
    
    regVal |= TMP102_MASK_CFG_OS;
    setConfig(regVal);
}


/*
 *  Checks to determine if a one-shot conversion has completed.  Returns 1 if
 *  one-shot bit is clear. Returns 0 otherwise.
 */
int TMP102::oneShotReady()
{
    int32_t regVal = getConfig();
    
    if (regVal & TMP102_MASK_CFG_OS)
        return 1;
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