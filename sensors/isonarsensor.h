#ifndef ISONARSENSOR_H
#define ISONARSENSOR_H

/** @brief Virtual interface class for any type of sonar sensor.
 *
 *  Class to describe an interface to a wide varity of sonar range sensors.
 *  Specfic types of sensors can implement this interface to allow other code 
 *  to interface to several types of sensor without needing to know the specfics 
 *  of the type of sensor.  The trigger function is expected to return the 
 *  results of the ranging event as time-of-flight in microseconds.
 */
class ISonarSensor
{
public:
    virtual ~ISonarSensor() {};

    virtual long trigger();
    virtual long getRange_in();
    virtual long getRange_cm();
    virtual int setEnable(int e);
    virtual int isEnabled();

protected:
    int enabled;                                
};








/** @brief Trigger the sensor to start a ranging session
 *
 *  This function must be overidden to implement the actual code needed to
 *  cause a sensor to perform a ranging cycle.  The ranging cycle should be
 *  completed when this function returns and either a positive value returned
 *  or a negative value returned for failure.
 *
 *  @return int: The time of flight in microseconds
 *
 */
inline long ISonarSensor::trigger()
{
    return -1;
}


/** @brief Get the range in inches from the last ranging cycle
 *
 *  This function must be overidden to provide the distance to the target in
 *  tenths of an inch.  Returns negative number on failure.
 *
 *  @return int: Range to target in tenths of an inch (ie 10.5" = 105)
 */
inline long ISonarSensor::getRange_in()
{
    return -1;
}


/** @brief Get the range in centimeters from the last ranging cycle
 *
 *  This function must be overidden to provide the distance to the target in
 *  centimeters.  Returns negative number on failure.
 *
 *  @return int: Range to target in centimeters
 */
inline long ISonarSensor::getRange_cm()
{
    return -1;
}


/** @brief Enable or disable the sensor object
 *
 *  This will enable or disable the sensor object.  Disabled sensors will not 
 *  perform any hardware activity when triggered and will always return -1 for
 *  values when requesting range data.
 *
 *  @param  int e: 0 for disable or 1 for enable
 *  @return int: The new enable status
 *
 */
inline int ISonarSensor::setEnable(int e)
{
    if (e)
        enabled = 1;
    else
        enabled = 0;
    
    return enabled;
}


/** @brief Return the enable status of the sensor
 *
 *  @return int: Enable status
 */
inline int ISonarSensor::isEnabled()
{
    return enabled;
}


#endif // ISONARSENSOR_H
