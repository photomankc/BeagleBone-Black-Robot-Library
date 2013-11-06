//
//  itempsensor.h
//  TMP102 Demo
//
//  Created by Kyle Crane on 10/18/13.
//  Copyright (c) 2013 Kyle Crane. All rights reserved.
//

#ifndef TMP102_Demo_itempsensor_h
#define TMP102_Demo_itempsensor_h

class ITempSensor
{
protected:
    virtual ~ITempSensor(){};
    
    virtual int     isReady()=0;
    virtual float   getTemp_F()=0;
    virtual float   getTemp_C()=0;
    
    virtual int     setEnable(int val) {return enabled = val;};
    virtual int     isEnabled() {return enabled;};
    
protected:
    int enabled;
};

#endif
