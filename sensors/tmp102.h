#ifndef __tmp102__
#define __tmp102__

#include "i_i2c.h"
#include "itempsensor.h"

enum TMP102_CONST
{
    TMP102_DFLT_ADR = 0x48,
    TMP102_REG_TEMP = 0,
    TMP102_REG_CFG  = 1,
    TMP102_REG_TLOW = 2,
    TMP102_REG_THGH = 3,
    TMP102_FQ_1 = 0,
    TMP102_FQ_2 = 1,
    TMP102_FQ_4 = 2,
    TMP102_FQ_6 = 3,
    TMP102_CR_1_4 = 0x00,
    TMP102_CR_1 = 0x40,
    TMP102_CR_4 = 0x80,
    TMP102_CR_8 = 0xC0,
    TMP102_LSB = 1,
    TMP102_MSB = 0,
    TMP102_CACHE_FORCE = 1,
    TMP102_CACHE_INVALID = -1,
    TMP102_ERR_TEMP = -1024,
    TMP102_ERR_BUS  = -1025,
    TMP102_ERR_NRDY = -1026,
};

enum TMP102_CFG_MASKS
{
    TMP102_MASK_CFG_SD  = 0x0100,
    TMP102_MASK_CFG_TM  = 0x0200,
    TMP102_MASK_CFG_POL = 0x0400,
    TMP102_MASK_CFG_FQ  = 0x1800,
    TMP102_MASK_CFG_R   = 0x6000,
    TMP102_MASK_CFG_OS  = 0x8000,
    TMP102_MASK_CFG_CR  = 0x00C0,
    TMP102_MASK_CFG_AL  = 0x0020,
    TMP102_MASK_CFG_EM  = 0x0010
};


/** @brief Class to represent the TMP102 I2C temperature measurement chip
 *
 *  This class represents the TMP102 I2C temperature measurement chip basic
 *  operation.  This object requires access to an I2C bus that implements the 
 *  I_I2C interface class.  TMP102 can use either a stack allocated and 
 *  externaly managed I2C class or it can use a dynamicaly allocated I2C object 
 *  for which it will manage the memory clean-up.  This class implements both
 *  the continuous mode and one-shot mode of operation for the sensor.  None of
 *  the chip thermostat features are implemented.
 *
 */
class TMP102 : public ITempSensor
{
public:
    TMP102(I_I2C* bus, uint8_t address=TMP102_DFLT_ADR);
    TMP102(I_I2C& bus, uint8_t address=TMP102_DFLT_ADR);
    ~TMP102();

    int         isReady();
    float       getTemp_F();
    float       getTemp_C();
    int         setEnable(int val);

    int32_t     getConfig(int force=0);
    void        setConfig(uint16_t cfg);
    void        setOneShot(uint16_t val);
    int         getOneShot();
    void        setConversionRate(uint16_t val);
    int         getConversionRate();

private:
    int32_t     read(int16_t reg);
    int         write(int16_t reg, uint16_t val);
    void        triggerOneShot();
    int         oneShotReady();
    
protected:
    uint8_t     adr;
    int32_t     cfgCache;
    I_I2C*      p_bus;
    int         ownBus;
    int         oneShotActive;
    int         oneShotTrigger;
};


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

#endif /* defined(__tmp102__) */
