#ifndef FS_I2C_H
#define FS_I2C_H


#include <i2c-dev.h>
#include "i_i2c.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>


class FS_I2C : public I_I2C
{
private:
    int 	m_file;
    char* 	m_fname;

public:
    FS_I2C(char const* fname);
    virtual ~FS_I2C();

    int openBus(uint8_t slaveAdr);
    int closeBus();
	int isReady();
    int tx(uint8_t* bytes, int count);
    int rx(uint8_t* bytes, int count);
    
    int tx(int32_t reg, uint8_t* bytes, int count);
    int txByte(uint8_t bt);
    int txByte(int32_t reg, uint8_t byte);
    int txWord(uint16_t wd);
    int txWord(int32_t reg, uint16_t wd);
    
    int        rx(int32_t reg, uint8_t* bytes);
    int8_t     rxByte();
    int8_t     rxByte(int32_t reg);
    int16_t    rxWord();
    int16_t    rxWord(int32_t reg);
};

#endif // I2C_H
