//
//  i_i2c.h
//  SFL6470
//
//  Created by Kyle Crane on 10/10/13.
//  Copyright (c) 2013 Kyle Crane. All rights reserved.
//

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
};

class I_I2C
{
public:
    virtual ~I_I2C() {};
    virtual int openBus(uint8_t slaveAdr)=0;
    virtual int closeBus()=0;
	virtual int isReady()=0;
    
    virtual int tx(uint8_t* bytes, int count)=0;
    virtual int txReg(uint8_t reg, uint8_t* bytes, int count)=0;
    virtual int txByte(uint8_t bt)=0;
    virtual int txByte(uint8_t reg, uint8_t byte)=0;
    virtual int txWord(uint16_t wd)=0;
    virtual int txWord(uint8_t reg, uint16_t wd)=0;
    
    virtual int         rx(uint8_t* bytes, int count)=0;
    virtual int         rxReg(uint8_t reg, uint8_t* bytes)=0;
    virtual uint8_t     rxByte()=0;
    virtual uint8_t     rxByte(uint8_t reg)=0;
    virtual uint16_t    rxWord()=0;
    virtual uint16_t    rxWord(uint8_t reg)=0;
    
protected:
    
};

#endif
