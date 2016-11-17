#ifndef CHRONODOT_H
#define CHRONODOT_H

#include "Clocks.h"
class chronodot
{
    public:
        chronodot();
        void writeClock(int address, byte hours, byte mins,byte secs);
        int getTemp();
        int alarmStatus();
        void getTime(int address,Clocks *_master);
        void enableAlarm( bool doEnable, bool alarm);

    private:
        void goToReg(int regNum);
};
#endif