#ifndef SFL6470_i_i2c_h
#define SFL6470_i_i2c_h

#include <stdint.h>

enum I2CERR // I2C Error codes for errno inspection.
{
	ERR_I2C_GEN	 = -1,
	ERR_I2C_BSY  = -2,
	ERR_I2C_FILE = -3,
	ERR_I2C_IO	 = -4,
	ERR_I2C_RNG  = -5,
    ERR_I2C_NOT_IMPL = -128
};

class I_I2C
{
public:
    virtual ~I_I2C() {};
    
    // Required overrides:
    virtual int openBus(uint8_t slaveAdr)=0;
    virtual int closeBus()=0;
	virtual int isReady()=0;
    virtual int tx(uint8_t* bytes, int count)=0;
    virtual int rx(uint8_t* bytes, int count)=0;
    
    
    // Optional overides
    virtual int devPresent() {return ERR_I2C_NOT_IMPL;};
    
    virtual int tx(int32_t reg, uint8_t* bytes, int count) {return ERR_I2C_NOT_IMPL;};
    virtual int txByte(uint8_t bt) {return ERR_I2C_NOT_IMPL;};
    virtual int txByte(int32_t reg, uint8_t byte) {return ERR_I2C_NOT_IMPL;};
    virtual int txWord(uint16_t wd) {return ERR_I2C_NOT_IMPL;};
    virtual int txWord(int32_t reg, uint16_t wd) {return ERR_I2C_NOT_IMPL;};
    virtual int txLong(uint32_t lg) {return ERR_I2C_NOT_IMPL;};
    virtual int txLong(uint32_t reg, uint32_t lg) {return ERR_I2C_NOT_IMPL;};
    
    virtual int        rx(int32_t reg, uint8_t* bytes)
                         {return ERR_I2C_NOT_IMPL;};
    virtual int8_t     rxByte() {return 0xFF;};
    virtual int8_t     rxByte(int32_t reg) {return 0xFF;};
    virtual int16_t    rxWord() {return 0xFFFF;};
    virtual int16_t    rxWord(int32_t reg) {return 0xFFFF;};
    virtual int32_t    rxLong() {return 0xFFFFFFFF;};
    virtual int32_t    rxLong(int32_t reg) {return 0xFFFFFFFF;};
    
protected:
    uint8_t m_adr;
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


#endif
