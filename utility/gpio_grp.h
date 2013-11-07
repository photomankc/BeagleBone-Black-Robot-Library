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
        map<int, GPIO_Pin*> m_pinStore;

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

#endif // GPIO_GRP_H
