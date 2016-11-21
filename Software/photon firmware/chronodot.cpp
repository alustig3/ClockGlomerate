#include "application.h"
#include "chronodot.h"
#include "Clock.h"


chronodot::chronodot(){
    Wire.begin();
}

void chronodot::goToReg(int regNum){
    Wire.beginTransmission(0x68);
    Wire.write((byte)regNum);
    Wire.endTransmission();
}

void chronodot::writeClock(int address, byte hours, byte mins, byte secs){
    Wire.beginTransmission(0x68); // address DS3231
    Wire.write(address); // select register
    Wire.write(secs/10<<4 | secs%10); // write register bitmap, bit 7 is /EOSC
    Wire.endTransmission();

    Wire.beginTransmission(0x68); // address DS3231
    Wire.write(address+1); // select register
    Wire.write(mins/10<<4 | mins%10);
    Wire.endTransmission();

    boolean isAfternoon = false;
    if (hours>11){
        hours = hours-12;
        isAfternoon = true;
    }
    if (hours==0){
        hours = 12;
    }
    Wire.beginTransmission(0x68); // address DS3231
    Wire.write(address+2); // select register
    Wire.write(1<<6 | isAfternoon<<5 | hours/10<<4 | hours%10);
    Wire.endTransmission();

    //set alarm register mask so M4,M3,M2,M1  = 1000 = Alarm occurs when hours, minutes and seconds match
    Wire.beginTransmission(0x68);
    Wire.write(address+3);
    Wire.write(1<<7);
    Wire.endTransmission();
}


int chronodot::alarmStatus(){
    goToReg(0x0F);
    Wire.requestFrom(0x68, 1);
    return Wire.read() & 3;
}


int chronodot::getTemp(){
    goToReg(0x11);
    Wire.requestFrom(0x68, 1);
    return Wire.read()*9/5.0 +32;
}

void chronodot::getTime(int address, Clock *_master){
    goToReg(address);
    Wire.requestFrom(0x68, 3); // request three bytes (seconds, minutes, hours)
    byte seconds = Wire.read(); // get seconds
    byte minutes = Wire.read(); // get minutes
    byte hours = Wire.read();   // get hours
    _master->first = (((hours & 0b00010000)>>4)*10 + (hours & 0b00001111)); // convert BCD to decimal (assume 24 hour mode)
    _master->isAfternoon = 1 & hours>>5;
    _master->second = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111)); // convert BCD to decimal
    _master->third = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111)); // convert BCD to decimal
}

void chronodot::enableAlarm(bool doEnable, bool alarm){ // 0/1 for disable/enable, 0/1 for alarm1/alarm2
    goToReg(0x0E);
    byte controlRegVal = Wire.requestFrom(0x68, 1);
    Wire.beginTransmission(0x68); // address DS3231
    Wire.write(0x0E); // select register
    if (doEnable){
        controlRegVal |= 1<<alarm;
    }
    else{
        controlRegVal &= ~(1<<alarm);
    }
    Wire.write(controlRegVal); // write register bitmap, bit 7 is /EOSC
    Wire.endTransmission();
}
