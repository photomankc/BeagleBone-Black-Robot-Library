#ifndef ADC_MCP3221_H_
#define ADC_MCP3221_H_

#include "i_i2c.h"

typedef enum MCP3221_CONSTANTS
{
    MCP3221_ADR_DFLT    = 0x4d,
    MCP3221_MAX_COUNT   = 4095,
    MCP3221_OBJ_ERR     = -255
} MCP3221_CONSTANTS;


class ADC_MCP3221
{
private:
    uint8_t     m_adr;
    I_I2C*		m_bus;
    int			m_countAvg;
    float		m_vref;
    float		m_vscale;
    int			m_cal;

public:
    ADC_MCP3221();
    ADC_MCP3221(I_I2C* bus, uint8_t adr = MCP3221_ADR_DFLT);
    ADC_MCP3221(I_I2C* bus, int cal, float vref);
    ADC_MCP3221(I_I2C* bus, int cal, float vref, float vscale);
    virtual ~ADC_MCP3221();

	void		init(I_I2C* p_bus, uint8_t adr);
	void		init(I_I2C* p_bus, int cal, float vref);
	void		init(I_I2C* p_bus, int cal, float vref, float vscale);
	void		set_cal(int val);
	void		set_vref(float val);
	void		set_vscale(float val);
	int			get_cal() {return m_cal;}
	float		get_vref() {return m_vref;}
	float		get_vscale() {return m_vscale;}

	int			update(int sCnt);
	int16_t     getCount() {return m_countAvg;}
	float		getVolts();
	float		getVolts(float vRef);
	float		getVolts(float vRef, float vScale);
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

#endif // ADC_MCP3221_H
