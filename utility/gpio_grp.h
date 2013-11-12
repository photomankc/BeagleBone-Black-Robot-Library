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

enum GPIO_GRP_OE
{
    GPIO_GRP_OE_CLOCK   = 1,
    GPIO_GRP_OE_ENABLE  = 0,
    GPIO_GRP_OE_POS     = 1,
    GPIO_GRP_OE_NEG     = 0
};


class GPIO_Grp
{
private:
    int m_nextBit;      /**<Bit position for next pin added. */
    int m_ownMgr;       /**<Flag for ownership of pin manager object. */
    int m_grpDir;       /**<I/O direction of the pin group. */
    int m_oePol;        /**<Polarity of output enable. */
    int m_oeAct;        /**<Clocked output vs enable/disable. */
    GPIO_Pin* m_oePin;  /**<Output enable pin. */
    BBGPIO_Mgr* m_pMgr;
    std::map<int, GPIO_Pin*> m_pinStore;

public:
    GPIO_Grp();
    GPIO_Grp(BBGPIO_Mgr &mgr);
    virtual ~GPIO_Grp();
    void init();
    void init(BBGPIO_Mgr &mgr);
    int  addOutputEnable(int gpioNum, int pol, int act=GPIO_GRP_OE_ENABLE);
    void removeOutputEnable();
    int  set_dir(int dir);
    void set(int val=GPIO_HIGH);
    void set(int bitPos, int val);
    int  get();
    int  get(int bitPos);
    int  addPin(int gpioNum);
    int  removePin(int bitPos);

protected:
};


/** @brief Create a GPIO_Grp object that uses it's own pin manager.
 *
 *  TODO: This will need help since ideally it should be possible to use many
 *  different platforms.  How to create the correct pin manager?
 */
inline GPIO_Grp::GPIO_Grp()
{
    init();
}

/** @brief Create a GPIO_Grp object with an existing pin manager.
 *
 *  TODO: Abstract the manager so this is not dependent on the platform.
 *
 *  @param BBGPIO_Mgr&: Reference to existing pin manager.
 */
inline GPIO_Grp::GPIO_Grp(BBGPIO_Mgr& mgr)
{
    init(mgr);
}


/** @brief Destroy the GPIO_Grp object
 *
 */
inline GPIO_Grp::~GPIO_Grp()
{
    removeOutputEnable();
    if (m_ownMgr)
        delete m_pMgr;
}



/** @brief Initialize the GPIO group with internal pin manager.
 *
 * (documentation goes here)
 */
inline void GPIO_Grp::init()
{
    m_pMgr = new BBGPIO_Mgr;
    m_ownMgr = 1;
    m_nextBit = 0;
    m_oePin = 0;
}



/** @brief Initialize the GPIO group with external pin manager.
 *
 *  Creates the group object and assigns the specified external pin manager.
 *  the group will not delete this manager upon destruction, it is assumed to
 *  handled by external entities.
 *  
 *  @param BBGPIO_mgr&: Reference to external pin manager.
 */
inline void GPIO_Grp::init(BBGPIO_Mgr& mgr)
{
    m_pMgr = &mgr;
    m_ownMgr = 0;
    m_nextBit = 0;
    m_oePin = 0;
}


/** @brief Add an output enable signal to the group.
 *
 *  This adds a pin to be used as an output enable signal to allow valid data to
 *  be indicated at the end of a set operation.  Since a set involves a sequence
 *  of individual set opeations on each pin it is possible for a system to 
 *  sample in valid data durring that operation.  This allows either a blanking 
 *  period during the operation, or a clocking pulse at the end of the operation
 *  to signal that the operation is complete.
 *
 *  @param gpioNum: The GPIO pin to use for this function
 *  @param pol: The polarity of the signal 0 - Active Low | 1 - Active High
 *  @param act: The type of OE action 0 - Output Enable | 1 - Clock Pulse
 */
inline int GPIO_Grp::addOutputEnable(int gpioNum, int pol, int act)
{
    GPIO_Pin *pPin;
    
    pPin = m_pMgr->aquirePin(gpioNum);
    if (pPin == NULL)
        return m_pMgr->getLastErr();
    else
    {
        std::cout << "GPIO_Grp::addOutputEnable() - Added Output Enable." << std::endl;
        m_oePin = pPin;
        m_oePin->set_dir(GPIO_OUT);
        
        if (pol)
            m_oePol = GPIO_GRP_OE_POS;
        else
            m_oePol = GPIO_GRP_OE_NEG;
        
        if (act)
            m_oeAct = GPIO_GRP_OE_CLOCK;
        else
            m_oeAct = GPIO_GRP_OE_ENABLE;
        
        // Disable OE since no valid data is presented
        if (m_oePol == GPIO_GRP_OE_POS)
            m_oePin->set(GPIO_LOW);
        else
            m_oePin->set(GPIO_HIGH);
        
        return GPIO_OK;
    }
}


/** @brief Remove the output enable signal pin from the group.
 *
 */
inline void GPIO_Grp::removeOutputEnable()
{
    if (m_oePin)
    {
        m_pMgr->releasePin(m_oePin);
        m_oeAct = 0;
        m_oePol = 0;
    }
}


/** @brief Set the I/O direction for all pins in the group.
 *
 *  @param dir: The I/O direction to set for all group pins.
 *  @return int: Result code.  Negative numbers on error.
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


/** @brief Set the pin group pins to pattern matching the given value.
 *
 *  Writes the value to the pin group as a bit pattern.  The pattern is written
 *  to the GPIO pins starting at the LSB and progressing towards the MSB.  Data
 *  beyond the number of pins in the group is discarded.  If an output enable
 *  pin is specified then it is deactivated prior to writing the pin values and
 *  reactivated once all pins are set.  This allows for clocking data only when
 *  all bits have been writen.
 *
 *  @param val: Value representing the bit pattern to write to the group.
 */
inline void GPIO_Grp::set(int val)
{
    //std::cout << "    GPIO_Grp.set(" << val << ")" << std::endl;
    int mask = 0;
    int bitVal = 0;
    
    if (m_pinStore.empty())
        return;
    
    if (m_grpDir == 0)
        return;
    
    if (m_oePin)
    {   // Disable OE pin while data is being changed.
        if (m_oeAct == GPIO_GRP_OE_ENABLE)
        {
            if (m_oePol)
                m_oePin->set(0);
            else
                m_oePin->set(1);
        }
    }
    
    for (std::map<int,GPIO_Pin*>::iterator it=m_pinStore.begin(); it!=m_pinStore.end(); ++it)
    {
        mask = 1 << it->first;  // Construct mask for this bit position
        bitVal = (val & mask) >> it->first; // AND that against the value specified and return to bit 0.
        //std::cout << "      m:" << mask << "  v:" << bitVal << std::endl;
        it->second->set(bitVal); // Set the GPIO_Pin's value to the result
    }
    
    if (m_oePin)
    {
        if (m_oeAct == GPIO_GRP_OE_ENABLE)
        {   // Enable OE pin again now that data is set.
            if (m_oePol)
                m_oePin->set(1);
            else
                m_oePin->set(0);
        }
        else
        {   // Generate a data clock signal on OE pin with correct polarity.
            if (m_oePol)
            {
                m_oePin->set(1);
                m_oePin->set(0);
            }
            else
            {
                m_oePin->set(0);
                m_oePin->set(1);
            }
        }
    }
}


/** @brief Set the value of a specific bit in the group.
 *
 *  @param bitPos: The bit position to set the value of.
 *  @param val: The value to set this bit to.
 */
inline void GPIO_Grp::set(int bitPos, int val)
{
    std::map<int,GPIO_Pin*>::iterator it;
    
    it = m_pinStore.find(bitPos);
    if (it != m_pinStore.end())
    {
        it->second->set(val);
    }
}


/** @brief Returns a value that represents the bit pattern of all pins.
 *
 *  @return int: Value of all pins in the group.
 */
inline int GPIO_Grp::get()
{
    int result = 0;
    
    for (std::map<int,GPIO_Pin*>::iterator it=m_pinStore.begin(); it!=m_pinStore.end(); ++it)
    {
        result |= (it->second->get() << it->first);
    }
    return 0;
}


/** @brief Returns the value of a specific bit in the group.
 *
 *  @param bitPos: The bit position to fetch the value of.
 */
inline int GPIO_Grp::get(int bitPos)
{
    std::map<int,GPIO_Pin*>::iterator it;
    
    it = m_pinStore.find(bitPos);
    if (it != m_pinStore.end())
    {
        return it->second->get();
    }
    else
        return 0;
}


/** @brief Add a pin to the group for the given GPIO number.
 *
 *  Aquires a GPIO pin for the given GPIO number and adds it to the pin group
 *  at the next open bit position.  
 *
 *  @param gpioNum: The GPIO pin number to add.
 *  @return int: Result code.  Negative numbers on error.
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


/** @brief Remove a pin from the group at the given bit position.
 *
 *  Removes the pin from the group and releases it with the pin manager.  All
 *  pins below the current pin in the bit order are shifted up to fill the gap.
 *  This is an expensive operation and should not be done repeatedly.
 *
 *  @param bitPos: The bit position in the group to remove.
 *  @return int: Result code. Negative on error.
 */
inline int GPIO_Grp::removePin(int bitPos)
{
    std::map<int,GPIO_Pin*>::iterator it;
    int result = 0;
    
    it = m_pinStore.find(bitPos);
    if(it != m_pinStore.end())
    {
        result = m_pMgr->releasePin(it->second);
        if (result < 0)
            return GPIO_GENERR;
        
        result = m_pinStore.erase(bitPos);
        
        // TODO: Need to reorder the list to collapse the bit numbers to account
        // for the now missing bit.  Remove, add re-add all elements below this
        // bit position.
        
        if (result < 0)
            return GPIO_GENERR;
        else
            return GPIO_OK;
    }
    else
        return GPIO_GENERR;
}





#endif // GPIO_GRP_H
