#include "gpio_grp.h"

GPIO_Grp::GPIO_Grp()
{
    init();
}

GPIO_Grp::GPIO_Grp(BBGPIO_Mgr &mgr)
{
    init(mgr);
}

GPIO_Grp::~GPIO_Grp()
{
    if (m_ownMgr)
        delete m_pMgr;
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
void GPIO_Grp::init()
{
    m_pMgr = new BBGPIO_Mgr;
    m_ownMgr = 1;
    m_nextBit = 0;
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
void GPIO_Grp::init(BBGPIO_Mgr &mgr)
{
    m_pMgr = &mgr;
    m_ownMgr = 0;
    m_nextBit = 0;
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
int GPIO_Grp::set_dir(int dir)
{
    int result = 0;
    if (m_pinStore.empty())
        return GPIO_GENERR;

    // Set the direction of all pins in the group
    for (map<int,GPIO_Pin*>::iterator it=m_pinStore.begin(); it!=m_pinStore.end(); ++it)
        result |= it->second->set_dir(dir);

    m_grpDir = dir;
    return result;
}


void GPIO_Grp::set(int val)
{
    cout << "    GPIO_Grp.set(" << val << ")" << endl;
    int mask = 0;
    int bitVal = 0;

    if (m_pinStore.empty())
        return;

    if (m_grpDir == 0)
        return;

    for (map<int,GPIO_Pin*>::iterator it=m_pinStore.begin(); it!=m_pinStore.end(); ++it)
    {
        mask = 1 << it->first;                  // Construct mask for this bit position
        bitVal = (val & mask) >> it->first;     // AND that against the value specified and return to bit 0.
        cout << "      m:" << mask << "  v:" << bitVal << endl;
        it->second->set(bitVal);                // Set the GPIO_Pin's value to the result
    }
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
int GPIO_Grp::addPin(GPIO_Pin* pPin)
{
    return addPin(pPin->get_GPIONum());
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
int GPIO_Grp::addPin(int gpioNum)
{
    GPIO_Pin *pPin;

    pPin = m_pMgr->aquirePin(gpioNum);
    if (pPin == NULL)
        return m_pMgr->getLastErr();

    m_pinStore.insert(pair<int,GPIO_Pin*>(m_nextBit, pPin));
    cout << "    GPIO_Grp.addPin(" << gpioNum << ")  at bit:" << m_nextBit << endl;
    m_nextBit++;
    return 0;
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
void GPIO_Grp::deletePin(int bitPos)
{
}



/** @brief (one liner)
  *
  * (documentation goes here)
  */
void GPIO_Grp::deletePin(GPIO_Pin *pPin)
{
}

