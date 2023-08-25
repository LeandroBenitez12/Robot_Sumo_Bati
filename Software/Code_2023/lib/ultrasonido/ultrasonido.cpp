#include "ultrasonido.h"
#include "Arduino.h"

Ultrasonido::Ultrasonido(int trig, int echo)
{
    pin_trig = trig;
    pin_echo = echo;

    pinMode(pin_echo, INPUT);
    pinMode(pin_trig, OUTPUT);
    digitalWrite(pin_trig, LOW);
}

float Ultrasonido::sensorRead()
{
    float distancia;
    float ultrasonidoPulso;
    digitalWrite(pin_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_trig, LOW);
    ultrasonidoPulso = pulseIn(pin_echo, HIGH);
    distancia = ultrasonidoPulso / 58.2;
    return distancia;
}