#ifndef SOMO_H
#define SOMO_H

class SOMO{
    public:
        SOMO();
        void send(unsigned char CMD, unsigned char para1=0 , unsigned char para2 =0);
        unsigned char track = 0x0F;
        unsigned char play = 0x0D;
        unsigned char stop = 0x16;
        unsigned char volume = 0x06;
        unsigned char next = 0x01;
        unsigned char randOrder = 0x18;
        unsigned char reset = 0x0C;

    private:
        unsigned char message[8] = {0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};

};
#endif
