#ifndef CLOCK_H
#define CLOCK_H

class Clock{
    public:
        int first,second,third;
        byte aux;
        bool isAfternoon;
        byte digits[6] = {10,10,10,10,10,10};

        Clock();
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
