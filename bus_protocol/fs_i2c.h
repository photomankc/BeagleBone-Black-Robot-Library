#ifndef FS_I2C_H
#define FS_I2C_H


#include "i2c-dev.h"
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
    int txReg(uint8_t reg, uint8_t* bytes, int count);
    int txByte(uint8_t bt);
    int txByte(uint8_t reg, uint8_t byte);
    int txWord(uint16_t wd);
    int txWord(uint8_t reg, uint16_t wd);

    int         rx(uint8_t* bytes, int count);
    int         rxReg(uint8_t reg, uint8_t* bytes);
    uint8_t     rxByte();
    uint8_t     rxByte(uint8_t reg);
    uint16_t    rxWord();
    uint16_t    rxWord(uint8_t reg);
};

#endif // I2C_H
