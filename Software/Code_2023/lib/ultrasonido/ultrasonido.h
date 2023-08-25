#ifndef _ULTRASONIDO_H
#define _ULTRASONIDO_H
#include "Arduino.h"

class Ultrasonido

{
private:
    int pin_trig;
    int pin_echo;

public:
    
    Ultrasonido(int trig, int echo);
    float sensorRead();
};

#endif