////////////////////////////////////////////////////////////////////////////////////////////////
// ADC_MCP3221.h
//
// Object representing the single channel I2C 12bit Analog to Digital conversion chip.  This
// object provides a simple interfece to get single samples or muliple averaged samples.  The
// object will also calculate the voltage at the input based on the known full-scale reference
// voltage.
//

#ifndef ADC_MCP3221_H_
#define ADC_MCP3221_H_

#include "i2c.h"

typedef enum MCP3221_CONSTANTS
{
    MCP3221_ADR_DFLT = 0x4d,
    MCP3221_MAX_COUNT = 4095,
    MCP3221_OBJ_ERR		 = -2
} MCP3221_CONSTANTS;


class ADC_MCP3221
{
private:
    uint8_t	m_addr;
    I2C*		m_bus;
    int			m_countAvg;
    float		m_vref;
    float		m_vscale;
    int			m_cal;

public:
    ADC_MCP3221();
    ADC_MCP3221(I2C& bus);
    ADC_MCP3221(I2C& bus, uint8_t addr);
    ADC_MCP3221(I2C& bus, uint8_t addr, int cal, float vref);
    ADC_MCP3221(I2C& bus, uint8_t addr, int cal, float vref, float vscale);
    virtual ~ADC_MCP3221();

    void		init(I2C& bus);
	void		init(I2C& bus, uint8_t addr);
	void		init(I2C& bus, uint8_t addr, int cal, float vref);
	void		init(I2C& bus, uint8_t addr, int cal, float vref, float vscale);
	void		set_cal(int val);
	void		set_vref(float val);
	void		set_vscale(float val);
	int			get_cal() {return m_cal;}
	float		get_vref() {return m_vref;}
	float		get_vscale() {return m_vscale;}

	int			update(int sCnt);
	int16_t	getCount() {return m_countAvg;}
	float		getVolts();
	float		getVolts(float vRef);
	float		getVolts(float vRef, float vScale);
};

#endif // ADC_MCP3221_H
