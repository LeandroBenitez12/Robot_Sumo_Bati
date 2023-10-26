#include <DistanceSensors.h>
#include "BluetoothSerial.h"

#define PIN_ECHO_DER  33
#define PIN_TRIGG_DER 25
#define PIN_TRIGG_IZQ 32
#define PIN_ECHO_IZQ 35
double distance_izq;
double distance_der;

unsigned long currentTime = 0;
#define TICK_DEBUG 500

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
Isensor *ultrasoundDer = new Ultrasound(PIN_TRIGG_DER, PIN_ECHO_DER);
Isensor *ultrasoundIzq = new Ultrasound(PIN_TRIGG_IZQ, PIN_ECHO_IZQ);

void setup() {
  Serial.begin(9600);
  SerialBT.begin("Bati");
}

void loop() {
  distance_izq = ultrasoundDer->SensorRead();
  distance_der = ultrasoundIzq->SensorRead();

  if (millis() > currentTime + TICK_DEBUG)
  {
    SerialBT.print("Distance_izq: ");
    SerialBT.println(distance_izq);
    SerialBT.print("Distance_der");
    SerialBT.println(distance_der);
  }
}
