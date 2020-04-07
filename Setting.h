

//*****************************************************
//  Virtual pins

//#define VPIN_LabelValu V0
#define VPIN_TimeInput V1
#define VPIN_Button_Manual_Auto V2
#define VPIN_Button_On_Off_Relay5 V5
#define VPIN_Button_On_Off_Relay6 V6



//*****************************************************
//  Physical pins

  #define PIN_Relay7 7 // Build-in relay - PIN 7 NANO
  #define PIN_Relay8 8 // Build-in relay - PIN 8 NANO
  #define PIN_Relay5 5 // Relay module - PIN 5 NANO
  #define PIN_Relay6 6 // Relay module-  PIN 6 NANO
  #define pinACsensor 12 // AC sensor


//*****************************************************


// variables
bool isFirstConnect = true;
bool Manual_Status = false; // Флаг включен ли ручной режим
bool Relay5Status = true;
bool Relay6Status = true;
//uint32_t lastAuto56ResetMillis = 0;
//uint32_t lastAuto56ResetSeconds = 0;
boolean ACState = false;
String ACNotify = "no info";
int averageFactor = 10;
float smokeValueAverage = 1;
float flameValueAverage = 1;
float coef = 1;
float bigVoltageValueAverage = 1;
float solarVoltageValueAverage = 1;
float acu1VoltageValueAverage = 1;
float acu2VoltageValueAverage = 1;
float optoModemVoltageValueAverage = 1;
float wifiRouterVoltageValueAverage = 1;
float humValueAverage = 1;
float tempValueAverage = 1;


            

// Переменные для TimerInput
uint32_t startsecondswd=86100;            // if we had no Blynk connection we swich off relays aftre 23h 55m from boot
uint32_t stopsecondswd=86400;             // день недели время остановки в секундах 24h 00m
uint32_t nowseconds=0;                // время в секундах с полуночи 
uint32_t clocksDiff =0;
