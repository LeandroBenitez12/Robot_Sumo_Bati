#include <AnalogWrite.h>
#include <DistanceSensors.h>
#include <boton.h>
#include <SSD1306.h>
#include <EngineController.h>
#include "BluetoothSerial.h"

//variable de pin de boton
#define PIN_BOTON 000

//variables para los pines de los motores
#define PIN_RIGHT_ENGINE_IN1 21
#define PIN_RIGHT_ENGINE_IN2 19
#define PIN_LEFT_ENGINE_IN1 22
#define PIN_LEFT_ENGINE_IN2 23
#define PWM_CHANNEL_RIGHT_IN1 1
#define PWM_CHANNEL_RIGHT_IN2 2
#define PWM_CHANNEL_LEFT_IN1 3
#define PWM_CHANNEL_LEFT_IN2 4

//variables para los pines de los ultrasonido
#define PIN_ECHO_DER  33
#define PIN_TRIGG_DER 25
#define PIN_TRIGG_IZQ 32
#define PIN_ECHO_IZQ 35
double Distance_der;
double Distance_izq;
unsigned long currentTime = 0;

//DEBUG
#define TICK_DEBUG_ULTRASONIDO 500

//Serial bluetooth
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

//Pantalla Oled
#define PIN_SDA 16
#define PIN_SCL 17

//Velocidades
#define SEARCH_SPEED 80
#define SPEED_MIN 100
#define SPEED_MED 180
#define SPEED_SEM_FAST 200
#define SPEED_FAST 255

//Distancias
#define DIST_MIN 10
#define DIST_MED 30
#define DIST_MAX 50

//Instancia Objetos
Isensor *ultrasoundDer = new Ultrasound(PIN_TRIGG_DER, PIN_ECHO_DER);
Isensor *ultrasoundIzq = new Ultrasound(PIN_TRIGG_IZQ, PIN_ECHO_IZQ);

Boton *start = new Boton(PIN_BOTON);

SSD1306 display(0x3C, PIN_SDA, PIN_SCL);

IEngine *rightEngine = new Driver_DRV8825(PIN_RIGHT_ENGINE_IN1, PIN_RIGHT_ENGINE_IN2, PWM_CHANNEL_RIGHT_IN1, PWM_CHANNEL_RIGHT_IN2);
IEngine *leftEngine = new Driver_DRV8825(PIN_LEFT_ENGINE_IN1, PIN_LEFT_ENGINE_IN2, PWM_CHANNEL_LEFT_IN1, PWM_CHANNEL_LEFT_IN2);
EngineController *Bati = new EngineController(rightEngine, leftEngine);

//lectura del sensor 
void LecturaSensor()
{
    ultrasoundDer->SensorRead();
    ultrasoundIzq->SensorRead();
}

//Enum estrategias, posicion, etc

enum veniVeni
{
STAND_BY_VENI_VENI,
SEARCH_VENI_VENI,
TURN_RIGHT_VENI_VENI,
TURN_LEFT_VENI_VENI,
MODERATE_ATTACK_VENI_VENI,
ATTACK_VENI_VENI
};
int veniVeni = STAND_BY_VENI_VENI;

void VeniVeni()
{
    switch (veniVeni)
    {
    case STAND_BY_VENI_VENI:
    {
        Bati->Stop();
        display.clear();
        display.drawString(19, 0, "Strategy VeniVeni");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 28, "Press Star()");
        display.display();
        if (start->GetIsPress())
        {
            display.clear();
            display.drawString(19, 0, "Strategy VeniVeni");
            display.drawString(0, 9, "---------------------");
            display.drawString(0, 28, "Iniciando en 5");
            display.display();
            delay(5000);
            veniVeni = SEARCH_VENI_VENI;
        }
        break;
    }

        case SEARCH_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (ultrasoundDer <= DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (ultrasoundDer > DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (ultrasoundDer <= DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_RIGHT_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (ultrasoundDer > DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (ultrasoundDer > DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = SEARCH_VENI_VENI;
        if (ultrasoundDer <= DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_LEFT_VENI_VENI:
    {
        Bati->Left(SEARCH_SPEED, SEARCH_SPEED);
        if (ultrasoundDer <= DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (ultrasoundDer > DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = SEARCH_VENI_VENI;
        if (ultrasoundDer <= DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case MODERATE_ATTACK_VENI_VENI:
    {
        Bati->Stop();
        if (ultrasoundDer <= DIST_MIN && ultrasoundIzq <= DIST_MIN)
            veniVeni = ATTACK_VENI_VENI;
        if (ultrasoundDer > DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = SEARCH_VENI_VENI;
        if (ultrasoundDer <= DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (ultrasoundDer > DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = TURN_LEFT_VENI_VENI;
        break;
    }

    case ATTACK_VENI_VENI:
    {        
        Bati->Forward(SPEED_MIN, SPEED_MIN);
      
          if (ultrasoundDer > DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = SEARCH_VENI_VENI;
          if (ultrasoundDer <= DIST_MED && ultrasoundIzq > DIST_MED)
            veniVeni = TURN_RIGHT_VENI_VENI;
          if (ultrasoundDer > DIST_MED && ultrasoundIzq <= DIST_MED)
            veniVeni = TURN_LEFT_VENI_VENI;

        break;
    }
    }   
}

void SearchAtack()
{
    
}

enum posicionamiento
{
MENU_GIROS,
GIRO_0,
GIRO_45,
GIRO_90,
GIRO_135
};
int pocisionamiento = MENU_GIROS;

void posicionamiento()
{
    
};

enum estrategias
{
SELECT_ESTRATEGIA,
SELECT_POSICION,
SELECT_ESTATEGIA,
VENI_VENI,
OLD_SCHOOL,
FULL
};
int estrategias = SELECT_ESTRATEGIA;



