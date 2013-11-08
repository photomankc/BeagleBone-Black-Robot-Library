#ifndef BBGPIO_MGR_H
#define BBGPIO_MGR_H
#define DEBUG

/** BBGPIO_Mgr Class.
 *  Class to handle accessing and tracking use of BeagleBone GPIO pins.  The
 *  manager will create pin objects and maintain them in internal storage.
 *  Multiple manager objects can be created where needed but all will share
 *  the same storage.  The manager will check if the pin requested is muxed 
 *  properly and is not claimed by any other process.  All pin objects contained 
 *  are deleted when the final manager object goes out of scope.
 *
 *   @author     Kyle Crane
 *   @version    0.5.0
 */

#include <unordered_map>
#include <string.h>
#include "gpio_pin_defs.h"
#include "gpio_pin.h"
#include "fsgpio_pin.h"

#define BBGPIO_MGR_MODEFS   0
#define BBGPIO_MGR_MODEMEM  1
#define BBGPIO_MGR_PINCNT   65
#define BBGPIO_MGR_MAXGPIO  126


struct GPIO_PinData
{
    GPIO_Pin* m_pPin;   /**<GPIO_Pin pointer, pointer to GPIO_Pin object  */
    int m_refCnt;       /**<Integer, Reference count for this pin */
    int m_kernPin;      /**<Integer, Kernel pin number for this GPIO_Pin */
};

class BBGPIO_Mgr
{
    private:
        int m_mode;     /**<Integer, pin mode for pins that  */
        int m_id;       /**<Integer, manager ID number */
        int m_lastErr;  /**<Integer, error code from last operation */

        static int  ms_instanceID;
        static std::unordered_map<int, GPIO_PinData> ms_pinStore;

    public:
        BBGPIO_Mgr();
        BBGPIO_Mgr(int mode);
        virtual ~BBGPIO_Mgr();

        int         getID() {return m_id;};
        GPIO_Pin*   aquirePin(int headerNum);
        int         releasePin(GPIO_Pin *p_Pin);
        int         deletePin(int headerNum);
        int         deleteAll();
        int         getLastErr();

    private:
        int     lookupKernPin(int gpioNum);
        void    init();
};

#endif // BBGPIO_MGR_H
