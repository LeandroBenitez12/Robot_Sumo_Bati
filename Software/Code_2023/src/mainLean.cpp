//Librerias
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  //Oled
#include <EngineController.h>  //Motores
#include <AnalogSensor.h>      //libreria para sensores analogicos( sensores tatami)
#include <DistanceSensors.h>   //libreria para sensores
//#include <Button.h>
#include "BluetoothSerial.h"  //Bluetooh

//debug
#define DEBUG_SHARP 1
#define DEBUG_STATE 1
#define DEBUG_ANALOG 0
#define TICK_DEBUG_STATE 1500
#define TICK_DEBUG_ANALOG 1500
#define TICK_DEBUG_SHARP 1500
unsigned long currentTimeSharp = 0;
unsigned long currentTimeState = 0;
unsigned long currentTimeAnalog = 0;

int analog;
//Oled
#define SCREEN_WIDTH 128  // OLED width,  in pixels
#define SCREEN_HEIGHT 64  // OLED height, in pixels

//Pines sensores de Distancia
#define PIN_SHARP_LEFT 25
#define PIN_SHARP_CENTER_LEFT 33
#define PIN_SHARP_CENTER_RIGHT 32
#define PIN_SHARP_RIGHT 35

//Pines motores y canales PWM
#define PIN_MOTOR_IZQUIERDO_1 17
#define PIN_MOTOR_IZQUIERDO_2 16
#define PIN_MOTOR_DERECHO_1 26
#define PIN_MOTOR_DERECHO_2 27

#define CANAL_PWM_IZQUIERDO_1 1
#define CANAL_PWM_IZQUIERDO_2 2
#define CANAL_PWM_DERECHO_1 3
#define CANAL_PWM_DERECHO_2 4

// Pulsadores de Inicio y Estrategias
#define PIN_PULSADOR_START_1 5
#define PIN_PULSADOR_ESTRATEGIA_2 4
bool stateStart = 1;
unsigned long currentTimeButton = 0;
#define TICK_START 1000

// Variables de pantalla OLED
#define SCREEN_WIDTH 128  // OLED width,  in pixels
#define SCREEN_HEIGHT 64  // OLED height, in pixels
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Velocidades Sumo
#define VEL_MAX 255
#define VEL_BAJA 150
#define VEL_CORRECCION 90
#define VEL_GIRO 110
#define VEL_EJE_BUSQUEDA 90
#define VEL_GIRO_BUSQUEDA_MEJORADA_IZQ 70  // 130
#define VEL_GIRO_BUSQUEDA_MEJORADA_DER 60  // 100
// Variables distancia de sensores sharp
#define DIST_LECTURA_MAX 40  // sami = 35
int distSharpCenterLeft = 0;
int distSharpCenterRight = 0;
int distSharpLeft = 0;
int distSharpRight = 0;
class Sharp {
private:
  int pin;
  int n = 3;

public:
  Sharp(int p);
  double SharpDist();
};
Sharp::Sharp(int p) {
  pin = p;
  pinMode(pin, INPUT);
}
double Sharp::SharpDist() {
  long suma = 0;
  for (int i = 0; i < n; i++)  // Realizo un promedio de "n" valores
  {
    suma = suma + analogRead(pin);
  }
  float adc = suma / n;
  // float distancia_cm = 17569.7 * pow(adc, -1.2062);
  if (adc < 400)
    adc = 400;
  // float distancia_cm = 2076.0 / (adc - 11.0);
  // Formula para el sensor GP2Y0A60SZLF
  // https://www.instructables.com/How-to-setup-a-Pololu-Carrier-with-Sharp-GP2Y0A60S/
  double distancia_cm = 187754 * pow(adc, -1.183);  //REGULAR LA POTENCIA PARA OBETENER BUENA PRESICION
  return (distancia_cm);
  delay(100);
}

//configuramos el Serial Bluetooth
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

// --- Sensores Centro
Sharp *sharpCenterLeft = new Sharp(PIN_SHARP_CENTER_LEFT);
Sharp *sharpCenterRight = new Sharp(PIN_SHARP_CENTER_RIGHT);
// --- Sensores Costados
Sharp *sharpLeft = new Sharp(PIN_SHARP_LEFT);
Sharp *sharpRight = new Sharp(PIN_SHARP_RIGHT);

//Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

IEngine *rightEngine = new Driver_DRV8825(PIN_MOTOR_DERECHO_1, PIN_MOTOR_DERECHO_2, CANAL_PWM_DERECHO_1, CANAL_PWM_DERECHO_2);
IEngine *leftEngine = new Driver_DRV8825(PIN_MOTOR_IZQUIERDO_1, PIN_MOTOR_IZQUIERDO_2, CANAL_PWM_IZQUIERDO_1, CANAL_PWM_IZQUIERDO_2);
EngineController *Aldosivi = new EngineController(rightEngine, leftEngine);

//AnalogSensor *rightTatami = new AnalogSensor(PIN_SENSOR_TATAMI_DER);
//AnalogSensor *LeftTatami = new AnalogSensor(PIN_SENSOR_TATAMI_IZQ);

//Isensor *sharpRight = new Sharp_GP2Y0A02(PIN_SENSOR_DISTANCIA_DERECHO);
//Isensor *sharpLeft = new Sharp_GP2Y0A02(PIN_SENSOR_DISTANCIA_IZQUIERDO);

//Button *stat = new  Button(PIN_PULSADOR_START_1);
//Button *strategy = new  Button(PIN_PULSADOR_ESTRATEGIA_2, false);
bool flank = HIGH;
bool previousState;
void SetFlank(bool f) {
  flank = f;
  previousState = !flank;
}

bool GetIsPress() {
  /*
  bool actualState = digitalRead(PIN_PULSADOR_START_1);
  bool state = (previousState != actualState) && actualState == flank;
  previousState = actualState;
  delay(100);
  return state;
  */
  return digitalRead(PIN_PULSADOR_START_1);
}


//AnalogSensor *ldrSensor = new AnalogSensor(PIN_SENSOR_LDR);

void sharpReadings() {
  distSharpCenterLeft = sharpCenterLeft->SharpDist();
  distSharpCenterRight = sharpCenterRight->SharpDist();
  distSharpLeft = sharpLeft->SharpDist();
  distSharpRight = sharpRight->SharpDist();
  // Tatami?
}
//Funciones para imprimir las lecturas de los sensores por el serial Bluetooth
/*
char printAnalog(char t)
{
  if (millis() > currentTimeAnalog + TICK_DEBUG_ANALOG)
  {
    currentTimeAnalog = millis();
    SerialBT.print(t + " :");
    SerialBT.println(analog);
  }
}
*/
void printReadSensors() {
  if (millis() > currentTimeSharp + TICK_DEBUG_SHARP) {
    currentTimeSharp = millis();
    SerialBT.print("Left Distance: ");
    SerialBT.println(distSharpLeft);
    SerialBT.print("Left Center Distance: ");
    SerialBT.println(distSharpCenterLeft);
    SerialBT.print("right Center Distance: ");
    SerialBT.println(distSharpCenterRight);
    SerialBT.print("right Distance: ");
    SerialBT.println(distSharpRight);
    SerialBT.println("-----------------------");
  }
}

//Enum de estados de movimiento de robot
enum movimiento {
  INICIO,
  BUSQUEDA_MEJORADA,
  CORRECCION_IZQUIERDA,
  CORRECCION_DERECHA,
  TE_ENCONTRE_IZQUIERDA,
  TE_ENCONTRE_DERECHA,
  ATAQUE
};
// Variable que determina el movimiento del robot
int movimiento = INICIO;

void printStatus() {
  if (millis() > currentTimeState + TICK_DEBUG_STATE) {
    currentTimeState = millis();
    String state = "";
    switch (movimiento) {
      case INICIO:
        state = "INICIO";
        break;
      case BUSQUEDA_MEJORADA:
        state = "BUSQUEDA_MEJORADA";
        break;
      case CORRECCION_IZQUIERDA:
        state = "CORRECCION_IZQUIERDA";
        break;
      case CORRECCION_DERECHA:
        state = "CORRECCION_DERECHA";
        break;
      case TE_ENCONTRE_IZQUIERDA:
        state = "TE_ENCONTRE_IZQUIERDA";
        break;
      case TE_ENCONTRE_DERECHA:
        state = "TE_ENCONTRE_DERECHA";
        break;
      case ATAQUE:
        state = "ATAQUE";
        break;
    }

    SerialBT.print("State: ");
    SerialBT.println(state);
    SerialBT.print("|| boton : ");
    SerialBT.println(GetIsPress());
    SerialBT.println("---------");
  }
}

void switchCase() {
  switch (movimiento) {
    case INICIO:
      Aldosivi->Stop();
      // stateStart = GetIsPress();
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("INICIO");
      oled.setCursor(0, 9);
      oled.println("---------------------");
      oled.setCursor(0, 26);
      oled.println("Esperando confirmacion..");
      oled.display();

      while (GetIsPress() == true) {
        if (DEBUG_STATE) {
          if (millis() > currentTimeState + TICK_DEBUG_STATE) {
            currentTimeState = millis();
            SerialBT.println("Esperando confirmacion..");
          }
        }
      }
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Pressed");
      oled.display();
      if (DEBUG_STATE) SerialBT.println("Pressed");

      delay(4900);
      oled.clearDisplay();
      oled.display();

      movimiento = BUSQUEDA_MEJORADA;

      break;

    case BUSQUEDA_MEJORADA:
      // Busqueda sobre propio eje 
      Aldosivi->Left(VEL_EJE_BUSQUEDA, VEL_EJE_BUSQUEDA);

      // Busqueda moverse en circulo
      /*
      Aldosivi->Forward(VEL_GIRO_BUSQUEDA_MEJORADA_DER, VEL_GIRO_BUSQUEDA_MEJORADA_IZQ);
      */
      if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = ATAQUE;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX && distSharpLeft > DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = BUSQUEDA_MEJORADA;
      else if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX) movimiento = CORRECCION_IZQUIERDA;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = CORRECCION_DERECHA;
      else if (distSharpLeft <= DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_IZQUIERDA;
      else if (distSharpLeft > DIST_LECTURA_MAX && distSharpRight <= DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_DERECHA;
      break;

    case CORRECCION_IZQUIERDA:
      Aldosivi->Left(VEL_CORRECCION, VEL_CORRECCION);
      if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = ATAQUE;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX && distSharpLeft > DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = BUSQUEDA_MEJORADA;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = CORRECCION_DERECHA;
      else if (distSharpLeft <= DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_IZQUIERDA;
      else if (distSharpLeft > DIST_LECTURA_MAX && distSharpRight <= DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_DERECHA;
      break;

    case CORRECCION_DERECHA:
      Aldosivi->Right(VEL_CORRECCION, VEL_CORRECCION);
      if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = ATAQUE;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX && distSharpLeft > DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = BUSQUEDA_MEJORADA;
      else if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX) movimiento = CORRECCION_IZQUIERDA;
      else if (distSharpLeft <= DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_IZQUIERDA;
      else if (distSharpLeft > DIST_LECTURA_MAX && distSharpRight <= DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_DERECHA;
      break;

    case TE_ENCONTRE_IZQUIERDA:
      /* Giro para cronometrar
      Aldosivi->Left(VEL_GIRO_BUSQUEDA, VEL_GIRO_BUSQUEDA);
      delay(1500);
      */
      do {
        Aldosivi->Left(VEL_GIRO, VEL_GIRO);
      } while (sharpCenterRight->SharpDist() > DIST_LECTURA_MAX);
      if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = ATAQUE;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX && distSharpLeft > DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = BUSQUEDA_MEJORADA;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = CORRECCION_IZQUIERDA;
      else if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX) movimiento = CORRECCION_DERECHA;
      else if (distSharpLeft > DIST_LECTURA_MAX && distSharpRight <= DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_DERECHA;
      break;

    case TE_ENCONTRE_DERECHA:
      /* Giro para cronometrar
      Aldosivi->Right(VEL_GIRO, VEL_GIRO);
      delay(1500);
      */
      do {
        Aldosivi->Right(VEL_GIRO, VEL_GIRO);
      } while (sharpCenterLeft->SharpDist() > DIST_LECTURA_MAX);

      if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = ATAQUE;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX && distSharpLeft > DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = BUSQUEDA_MEJORADA;
      else if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX) movimiento = CORRECCION_IZQUIERDA;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = CORRECCION_DERECHA;
      else if (distSharpLeft <= DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_IZQUIERDA;
      break;

    case ATAQUE:
      Aldosivi->Forward(VEL_MAX, VEL_MAX);
      if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX && distSharpLeft > DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = BUSQUEDA_MEJORADA;
      else if (distSharpCenterLeft <= DIST_LECTURA_MAX && distSharpCenterRight > DIST_LECTURA_MAX) movimiento = CORRECCION_IZQUIERDA;
      else if (distSharpCenterLeft > DIST_LECTURA_MAX && distSharpCenterRight <= DIST_LECTURA_MAX) movimiento = CORRECCION_DERECHA;
      else if (distSharpLeft <= DIST_LECTURA_MAX && distSharpRight > DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_IZQUIERDA;
      else if (distSharpLeft > DIST_LECTURA_MAX && distSharpRight <= DIST_LECTURA_MAX) movimiento = TE_ENCONTRE_DERECHA;
      break;
      /*
    case 9:
      do {
        leftmotor->Backward(180);
        rightmotor->Forward(180);
      } while (sharpCenterRight->SharpDist() > DIST_LECTURA_MAX);

      leftmotor->Forward(180);
      rightmotor->Forward(180);
      break;

    case 15:
      do {
        leftmotor->Forward(180);
        rightmotor->Backward(180);
      } while (sharpCenterLeft->SharpDist() > DIST_LECTURA_MAX);

      leftmotor->Forward(180);
      rightmotor->Forward(180);
      break;

    case 16:
      // Busqueda del robot
      leftmotor->Forward(175);
      rightmotor->Forward(100);
      break;
*/
  }
}
void setup() {
  Serial.begin(115200);
  SerialBT.begin("Aldosivi");
  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(PIN_PULSADOR_START_1, INPUT_PULLUP);
}

void loop() {
  // Lectura de sharps
  sharpReadings();
  // Seleccion del movimiento del robot
  switchCase();

  if (DEBUG_SHARP) {
    printReadSensors();
  }

  if (DEBUG_STATE) {
    printStatus();
  }

  /*if (DEBUG_ANALOG)
  {
    printAnalog();
  }
  */
}
