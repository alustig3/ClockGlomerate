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

void chronodot::writeClock(int startAddress, char hours, char mins, char secs){
    //write to seconds register
    Wire.beginTransmission(0x68);
    Wire.write(startAddress);
    Wire.write(secs/10<<4 | secs%10);
    Wire.endTransmission();

    //write to minutes register
    Wire.beginTransmission(0x68);
    Wire.write(startAddress+1);
    Wire.write(mins/10<<4 | mins%10);
    Wire.endTransmission();

    //write to hours register
    boolean isAfternoon = false;
    if (hours>11){
        hours = hours-12;
        isAfternoon = true;
    }
    if (hours==0){
        hours = 12;
    }
    Wire.beginTransmission(0x68);
    Wire.write(startAddress+2);
    Wire.write(1<<6 | isAfternoon<<5 | hours/10<<4 | hours%10);
    Wire.endTransmission();

    //set alarm register mask so M4,M3,M2,M1  = 1000 = Alarm occurs when hours, minutes and seconds match
    Wire.beginTransmission(0x68);
    Wire.write(startAddress+3);
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

void chronodot::getTime(int address, Clock *_clock){
    goToReg(address);
    Wire.requestFrom(0x68, 3); // request three bytes (seconds, minutes, hours)
    byte seconds = Wire.read(); // get seconds
    byte minutes = Wire.read(); // get minutes
    byte hours = Wire.read();   // get hours
    _clock->first = (((hours & 0b00010000)>>4)*10 + (hours & 0b00001111));
    _clock->isAfternoon = 1 & hours>>5;
    _clock->second = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111));
    _clock->third = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111));
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
