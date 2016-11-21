#include "application.h"
#include "Clock.h"
#include "LedControl-MAX7219-MAX7221.h"
LedControl *lc;
uint8_t data = A5;
uint8_t load = A4;
uint8_t myclock = A3;
byte max1dig2[11][3] = {{254,198,254},  //zero
                        {0,0,254},      //one
                        {242,238,158},  //two
                        {146,238,254},  //three
                        {30,40,254},    //four
                        {158,238,242},  //five
                        {254,238,242},  //six
                        {2,6,254},      //seven
                        {254,238,254},  //eight
                        {30,46,254},    //nine
                        {0,0,0}};       //empty

Clock::Clock(){
    Wire.begin();
    lc = new LedControl(data,myclock,load,2); //DIN,CLK,CS,HowManyDisplay
    lc->shutdown(0,false);
    lc->shutdown(1,false);
    lc->setIntensity(0,15);  //set brightness
    lc->setIntensity(1,15);
    lc->clearDisplay(0);    //clear display
    lc->clearDisplay(1);
    for (int i = 0;i<8; i++){
        lc->setColumn(0,i,0);
        lc->setColumn(1,i,0);
    }
}
void Clock::displayAlt(){ //display hours minutes and 2 auxillary digits
    byte digsarr[6] = {(first/10==1)?1:10, first%10, second/10, second%10, aux/10, aux%10};
    // for (int i = 0; i<6; i++){
    //     Serial.print(digsarr[i]);
    //     Serial.print(",");
    // }
    // Serial.println();
    for(int col=0;col<3;col++) {
        if (col==2){                                        //digit 1
            lc->setColumn(0,0,max1dig2[digsarr[0]][col]);
        }
        lc->setColumn(0,col + 1,max1dig2[digsarr[1]][col]); //digit 2
        lc->setColumn(0,col + 4,max1dig2[digsarr[2]][col]); //digit 3
        if(col==0){                                         //digit 4
            lc->setColumn(0,col + 7 ,max1dig2[digsarr[3]][col]);
        }
        else{
            lc->setColumn(1,col - 1,max1dig2[digsarr[3]][col]);
        }
        lc->setColumn(1,col + 2,max1dig2[digsarr[4]][col]); //digit 5
        lc->setColumn(1,col + 5,max1dig2[digsarr[5]][col]); //digit 6
    }
}
void Clock::display(){ //display hours minutes seconds
    byte digsarr[6] = {(first/10==1)?1:10, first%10, second/10, second%10, third/10, third%10};
    // for (int i = 0; i<6; i++){
    //     Serial.print(digsarr[i]);
    //     Serial.print(",");
    // }
    // Serial.println();
    for(int col=0;col<3;col++) {
        if (col==2){                                        //digit 1
            lc->setColumn(0,0,max1dig2[digsarr[0]][col]);
        }
        lc->setColumn(0,col + 1,max1dig2[digsarr[1]][col]); //digit 2
        lc->setColumn(0,col + 4,max1dig2[digsarr[2]][col]); //digit 3
        if(col==0){                                         //digit 4
            lc->setColumn(0,col + 7 ,max1dig2[digsarr[3]][col]);
        }
        else{
            lc->setColumn(1,col - 1,max1dig2[digsarr[3]][col]);
        }
        lc->setColumn(1,col + 2,max1dig2[digsarr[4]][col]); //digit 5
        lc->setColumn(1,col + 5,max1dig2[digsarr[5]][col]); //digit 6
    }
}
void Clock::display(int number){ //display 6 digits of the same number
    for(int col=0;col<3;col++) {
        if (col==2){
            lc->setColumn(0,0,max1dig2[(number==1)?1:10][col]); //digit 1
        }
        lc->setColumn(0,col + 1,max1dig2[number][col]); //digit 2
        lc->setColumn(0,col + 4,max1dig2[number][col]); //digit 3
        if(col==0){                                         //digit 4
            lc->setColumn(0,col + 7 ,max1dig2[number][col]);
        }
        else{
            lc->setColumn(1,col - 1,max1dig2[number][col]);
        }
        lc->setColumn(1,col + 2,max1dig2[number][col]); //digit 5
        lc->setColumn(1,col + 5,max1dig2[number][col]); //digit 6
        delay(25);
    }
}
void Clock::display(int number,int digit){// update individual digit to specific number
    digits[digit-1] = number;
    for(int col=0;col<3;col++) {
        switch(digit){
        case 1:
            if (col==2){
                lc->setColumn(0,0,max1dig2[(number==1)?1:10][col]); //digit 1
            }
            break;
        case 2:
            lc->setColumn(0,col + 1,max1dig2[number][col]); //digit 2
            break;
        case 3:
            lc->setColumn(0,col + 4,max1dig2[number][col]); //digit 3
            break;
        case 4:
            if(col==0){                                         //digit 4
                lc->setColumn(0,col + 7 ,max1dig2[number][col]);
            }
            else{
                lc->setColumn(1,col - 1,max1dig2[number][col]);
            }
            break;
        case 5:
            lc->setColumn(1,col + 2,max1dig2[number][col]); //digit 5
            break;
        case 6:
            lc->setColumn(1,col + 5,max1dig2[number][col]); //digit 6
            break;
        }
        // delay(200);
    }
}
void Clock::set(char _first, char _second, char _third){
    first = _first;
    second = _second;
    third = _third;
}

void Clock::set(){
    first = digits[0]*10 + digits[1];
    second = digits[2]*10 + digits[3];
    third = digits[4]*10 + digits[5];
}

bool Clock::toggleDots(bool isDots){
    byte addend;
    if (isDots){
        addend = -16;
    }
    else {
        addend = 16;
    }
    for (int i = 0; i<12; i++){
        max1dig2[i][1] = max1dig2[i][1] + addend;
    }
    isDots=!isDots;
    return isDots;
}

void Clock::clearDisplay(){
    lc->clearDisplay(0);    //clear display
    lc->clearDisplay(1);
}
