#include "application.h"
#include "SOMO.h"

SOMO::SOMO(){
    Serial1.begin(9600);
    send(reset);
}

void SOMO::send(unsigned char CMD, unsigned char para1, unsigned char para2 ){
    // calculate checksum
    unsigned int checksum = 0xFFFF-(CMD+para1+para2) +1;
    message[1] = CMD;
    message[3] = para1;
    message[4] = para2;
    message[5] = checksum >> 8;
    message[6] = checksum & 255;
    Serial1.write(message,8);
}
