#ifndef CHRONODOT_H
#define CHRONODOT_H

#include "Clock.h"
class chronodot
{
    public:
        chronodot();
        void writeClock(int address, char hours, char mins,char secs);
        int getTemp();
        int alarmStatus();
        void getTime(int address,Clock *_master);
        void enableAlarm( bool doEnable, bool alarm);

    private:
        void goToReg(int regNum);
};
#endif
