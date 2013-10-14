#ifndef L6470_H
#define L6470_H

#include "l6470-support.h"
#include "ispi.h"
#include <stdint.h>


/** @brief Class to interface to the STI L6470 stepper motor driver chip
 *
 *  This class is designed to provide a code interface to the SPI based L6470
 *  Stepper Motor driver chip.  To use this device you will need to have 
 *  access to an SPI bus.  The object wraps the functionality of the SPI 
 *  commands and parameters to the chip.  This object requires you to provide
 *  it with a suitible SPI interface object that can be used to communicate to
 *  the hardware.  This class will not own the SPI object, but will expect
 *  exclusive use of it.  See the file "l6470-support.h" for a list of
 *  constants used extensively throughout the operation of the chip.
 *
 *  Be aware that most functions are non-blocking.  The chip performs the 
 *  needed stepping to move the motors. Functions that command motion will
 *  return immediately before that motion is completed.
 */
class L6470
{
protected:
    ISPI*   m_bus;
    int     m_ownBus;
    int     m_invertDir;
    int     m_msMode;

public:
    L6470(ISPI& bus, uint32_t cfg=0);
    L6470(ISPI* p_bus, uint32_t cfg=0);
    virtual ~L6470();
    void    initMotion(uint8_t microStp, float maxSpd = 500,
                       float acc=100, float dec=100);
    void    initBEMF(uint32_t k_hld, uint32_t k_mv, uint32_t int_spd, uint32_t st_slp,
                     uint32_t slp_acc);

    /********** Set Functions ************/
    void        setParam(uint8_t param, uint32_t value);
    uint32_t    setAccel(float spss);
    uint32_t    setDecel(float spss);
    uint32_t    setMaxSpeed(float sps);
    uint32_t    setSpeed(float sps);
    uint32_t    setFSCutOff(float sps); // TODO
    uint32_t    setINTSpeed(float sps); // TODO
    uint8_t     setMicroSteps(uint8_t val);
    int32_t     setPosition(int32_t);   // TODO
    int32_t     setPosition_FS(int32_t);// TODO
    void        setMark();              // TODO
    int         setConfig(uint32_t cfg);
    void        invert(uint8_t inv);

    /********** Get Functions ************/
    uint32_t    getParam(uint8_t param);
    uint32_t    isBusy();
    uint8_t     isInverted();
    uint32_t    getStatus();
    uint32_t    getConfig();
    uint8_t     getDir();           // TODO
    int32_t     getPosition();
    int32_t     getPosition_FS();
    uint32_t    getError();
    float       getAccel();
    float       getDecel();
    float       getMaxSpeed();
    float       getMinSpeed();
    float       getSpeed();
    float       getFSCutOff();      // TODO
    float       getINTSpeed();      // TODO
    uint8_t     getMicroSteps();
    
    /********** Device Commands ***********/
    void        resetDev();
    void        run(uint8_t dir, float spd);
    void        move(int32_t steps);
    void        move_FS(int32_t steps);
    void        gotoPosABS(int32_t pos);
    void        gotoPosABS_FS(int32_t pos);
    void        gotoPos(int32_t pos);
    void        gotoPos_FS(int32_t pos);
    void        goUntil(uint8_t act, uint8_t dir, float spd);
    void        releaseSW(uint8_t act, uint8_t dir);
    void        goHome();
    void        goMark();
    void        resetPos();
    void        softStop();
    void        hardStop();
    void        softHiZ();
    void        hardHiZ();

protected:
    uint32_t    paramHandler(uint8_t param, uint32_t value);
    uint32_t    procParam(uint32_t value, uint8_t bit_len);
    uint8_t     dspin_xfer(uint8_t data);
    uint8_t     dirInvert(uint8_t dir);
};


/** @brief If set to true, inverts the meaning of the direction bit.
  *
  * @inv - 0=Non-inverted | 1=Inverted
  */
inline void L6470::invert(uint8_t inv)
{
    if (inv) m_invertDir = 1;
    else m_invertDir = 0;
}


/** @brief Returns 1 if direction is inverted 0 otherwise.
 *
 *  @return uint8_t: Inverted setting
 */
inline uint8_t L6470::isInverted()
{
    return m_invertDir;
}


/** @brief Returns the number of microsteps per full step.
 *
 *  @return int: Microsteps per full physical motor step
 */
inline uint8_t L6470::getMicroSteps()
{
    return m_msMode;
}


/** @brief Returns the value stored in the chip configuration register
 *
 *  @return uint32_t: Register value containing all the configuration bits
 */
inline uint32_t L6470::getConfig()
{
    return getParam(dSPIN_CONFIG);
}


/** @brief Returns the current position stored in the chip as FULL STEPS
 *  
 *  This function returns the current position of the motor in full steps
 *  as a 32 bit integer.  Returns either a positive or negative number relative
 *  to the current 0 home position.
 *
 *  @return int32_t: Position value as signed value relative to home in full steps.
 */
inline int32_t L6470::getPosition_FS()
{
    float pos = (getPosition() / m_msMode) + 0.5;
    return (int32_t)pos;
}



/** @brief Move the motor the specified number of FULL STEPS and direction.
  *
  * Motor will accelerate, run to the relative number of FULL STEPS specified 
  * and in the direction specified, then decelerate.
  *
  * @param  dir Motor direction 0=REV | 1=FWD
  * @param  steps Number of FULL STEPS to move.
  */
inline void L6470::move_FS(int32_t steps)
{
    steps *= m_msMode;
    move(steps);
}


/** @brief Moves the motor to the specfied absolute position in FULL STEPS.
 *
 *  @param pos The absolute position to move to in FULL Steps
 */
inline void L6470::gotoPos_FS(int32_t pos)
{
    pos *= m_msMode;
    gotoPos(pos);
}


/** @brief Moves to the specfied absolute position in FULL STEPS in the direction specified.
 *
 *  @param pos The position to move to in FULL Steps
 */
inline void L6470::gotoPosABS_FS(int32_t pos)
{
    pos *= m_msMode;
    gotoPosABS(pos);
}


inline void L6470::releaseSW(uint8_t act, uint8_t dir)
{
    dspin_xfer(dSPIN_RELEASE_SW | act | dir);
}


/** @brief Return to the abs position 0 at MAX_SPEED in shortest path possible.
 *
 */
inline void L6470::goHome()
{
    dspin_xfer(dSPIN_GO_HOME);
}


/** @brief Goto positon in MARK register at MAX_SPEED in shortest distance
 *
 */
inline void L6470::goMark()
{
    dspin_xfer(dSPIN_GO_MARK);
}


/** @brief Reset the position counter to zero
 *
 */
inline void L6470::resetPos()
{
    dspin_xfer(dSPIN_RESET_POS);
}


/** @brief Reset the dSPIN chip to power on defaults
 *
 */
inline void L6470::resetDev()
{
    dspin_xfer(dSPIN_RESET_DEVICE);
}


/** @brief Halt using the deceleration curve.
 *
 */
inline void L6470::softStop()
{
    dspin_xfer(dSPIN_SOFT_STOP);
}


/** @brief Halt using infinite deceleration.
 *
 */
inline void L6470::hardStop()
{
    dspin_xfer(dSPIN_HARD_STOP);
}


/** @brief Halt using deceleration curve and put bridges in Hi-Z.
 *
 */
inline void L6470::softHiZ()
{
    dspin_xfer(dSPIN_SOFT_HIZ);
}


/** @brief Put the bridges in Hi-Z state immediately with no deceleration.
 *
 */
inline void L6470::hardHiZ()
{
    dspin_xfer(dSPIN_HARD_HIZ);
}

/*
 Copyright (C) 2013 Kyle Crane
 
 Adapted from example code for the SparkFun Breakout Board BOB-10859
 
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

#endif // SF-L6470_H

