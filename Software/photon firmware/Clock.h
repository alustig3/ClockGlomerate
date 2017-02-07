#ifndef CLOCK_H
#define CLOCK_H

class Clock{
    public:
        int first,second,third;
        char aux;
        bool isAfternoon;
        char digits[6] = {10,10,10,10,10,10};

        Clock();
        void displayAlt();
        void display();
        void countDown();
        void display(int number);
        void dispDigit(int number,int digit);
        void set(char _first, char _second, char _third);
        void set();
        bool toggleDots(bool isDots);
        void clearDisplay();
};
#endif
