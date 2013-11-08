#ifndef GPIO_GRP_H
#define GPIO_GRP_H

#include <map>
#include "bbgpio_mgr.h"

enum GPIO_GRP_BITS
{
    BIT_0 = 0x0001, BIT_1 = 0x0002,
    BIT_2 = 0x0004, BIT_3 = 0x0008,
    BIT_4 = 0x0010, BIT_5 = 0x0020,
    BIT_6 = 0x0040, BIT_7 = 0x0080,
    BIT_8 = 0x0100, BIT_9 = 0x0200,
    BIT_10 = 0x0400, BIT_11 = 0x0800,
    BIT_12 = 0x1000, BIT_13 = 0x2000,
    BIT_14 = 0x4000, BIT_15 = 0x8000
};


class GPIO_Grp
{
    private:
        int m_nextBit;
        int m_ownMgr;
        int m_grpDir;
        BBGPIO_Mgr* m_pMgr;
        std::map<int, GPIO_Pin*> m_pinStore;

    public:
        GPIO_Grp();
        GPIO_Grp(BBGPIO_Mgr &mgr);
        virtual ~GPIO_Grp();
        void init();
        void init(BBGPIO_Mgr &mgr);

        int  set_dir(int dir);
        void set(int val);
        int  get();
        int  addPin(int gpioNum);
        int  addPin(GPIO_Pin *pPin);
        void deletePin(int bitPos);
        void deletePin(GPIO_Pin *pPin);

    protected:
};

inline GPIO_Grp::GPIO_Grp()
{
    init();
}

inline GPIO_Grp::GPIO_Grp(BBGPIO_Mgr &mgr)
{
    init(mgr);
}

inline GPIO_Grp::~GPIO_Grp()
{
    if (m_ownMgr)
        delete m_pMgr;
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline void GPIO_Grp::init()
{
    m_pMgr = new BBGPIO_Mgr;
    m_ownMgr = 1;
    m_nextBit = 0;
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline void GPIO_Grp::init(BBGPIO_Mgr &mgr)
{
    m_pMgr = &mgr;
    m_ownMgr = 0;
    m_nextBit = 0;
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline int GPIO_Grp::set_dir(int dir)
{
    int result = 0;
    if (m_pinStore.empty())
        return GPIO_GENERR;
    
    // Set the direction of all pins in the group
    for (std::map<int,GPIO_Pin*>::iterator it=m_pinStore.begin(); it!=m_pinStore.end(); ++it)
        result |= it->second->set_dir(dir);
    
    m_grpDir = dir;
    return result;
}


inline void GPIO_Grp::set(int val)
{
    std::cout << "    GPIO_Grp.set(" << val << ")" << std::endl;
    int mask = 0;
    int bitVal = 0;
    
    if (m_pinStore.empty())
        return;
    
    if (m_grpDir == 0)
        return;
    
    for (std::map<int,GPIO_Pin*>::iterator it=m_pinStore.begin(); it!=m_pinStore.end(); ++it)
    {
        mask = 1 << it->first;                  // Construct mask for this bit position
        bitVal = (val & mask) >> it->first;     // AND that against the value specified and return to bit 0.
        std::cout << "      m:" << mask << "  v:" << bitVal << std::endl;
        it->second->set(bitVal);                // Set the GPIO_Pin's value to the result
    }
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline int GPIO_Grp::addPin(GPIO_Pin* pPin)
{
    return addPin(pPin->get_GPIONum());
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline int GPIO_Grp::addPin(int gpioNum)
{
    GPIO_Pin *pPin;
    
    pPin = m_pMgr->aquirePin(gpioNum);
    if (pPin == NULL)
        return m_pMgr->getLastErr();
    
    m_pinStore.insert(std::pair<int,GPIO_Pin*>(m_nextBit, pPin));
    std::cout << "    GPIO_Grp.addPin(" << gpioNum << ")  at bit:" << m_nextBit << std::endl;
    m_nextBit++;
    return 0;
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline void GPIO_Grp::deletePin(int bitPos)
{
}



/** @brief (one liner)
 *
 * (documentation goes here)
 */
inline void GPIO_Grp::deletePin(GPIO_Pin *pPin)
{
}

#endif // GPIO_GRP_H
