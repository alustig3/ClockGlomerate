#ifndef CLOCKS_H
#define CLOCKS_H

class Clocks{
    public:
        int first,second,third;
        int almSec,almMin,almHr;
        byte aux;
        bool isAfternoon;
        byte digits[6] = {10,10,10,10,10,10};

        Clocks();
        void displayAlt();
        void display();
        void display(int number);
        void display(int number,int digit);
        void set(char _first, char _second, char _third);
        void set();
        bool toggleDots(bool isDots);
        void clearDisplay();
};
#endif
