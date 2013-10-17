#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tmp102.h"

TMP102::TMP102(I_I2C* bus, uint8_t address)
{
    ownBus  = 0;
    p_bus   = 0;
    adr     = 0;
    
    if (bus)
    {
        p_bus = bus;
        ownBus = 1;
        adr = address;
    }
}


TMP102::~TMP102()
{
    if (ownBus)
    {
        delete p_bus;
    }
}


float TMP102::getTemp_F()
{
    return (getTemp_C() * 1.8) + 32;
}


float TMP102::getTemp_C()
{
    int err = 0;
    uint8_t bytes[2];
    int temperatureSum = 0;
    
    if (p_bus)
    {
        err = p_bus->openBus(adr);
        if (err == 0)
        {
            p_bus->rx(bytes, 2);
            p_bus->closeBus();
        }
        else
            return err;
    }
    
    temperatureSum = (bytes[0]<<8) | bytes[1];
    temperatureSum >>= 4;
    return temperatureSum * 0.0625;
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