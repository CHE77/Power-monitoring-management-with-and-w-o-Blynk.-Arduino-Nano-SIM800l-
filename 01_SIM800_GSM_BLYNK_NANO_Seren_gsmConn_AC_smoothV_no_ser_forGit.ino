/*****************************************************************************
   Power monitoring/management with and w/o Blynk. (Arduino Nano + SIM800l)
   Via Blynk we set period of time when to switch off power (via relay) for Modem and Router and switch on again.
   Blynk has bad habit to reset Nano if loose connection.
   So, we will try to connect Blynk only is we already have gsm coverage and internet.
   If it happens that Nano reboots and it could not connect to internet/Blynt it will
   switch off and switch on relays ones a day according preset time at sketch.
   Additionally Nano reads some sensors: smoke, flame, temperature and humidity,
   check presets (with homemade modules) of AC and measure DC with voltage divider shield.
   Sensors/Voltages readings go average before sending.

   Author: Aleksandr Kotov
   Modification of original project by:
   Автор: Обушенков Алексей Андреевич
   Группа в ВК https://vk.com/engineer24
   Канал на YouTube https://www.youtube.com/channel/UCih0Gcl9IEdkR8deDJCiodg
   Инженерка Engineering room

 *****************************************************************************

 *****************************************************************************

   Parts:
   http://geekmatic.in.ua/shield_arduino_nano_sim800_relay

   Arduino NANO CH340 USB

   SIM800  4,2V

*/



//*******************************************************
// SIM800L
#define BLYNK_PRINT Serial

// Select your modem:
#define TINY_GSM_MODEM_SIM800
//#define TINY_GSM_MODEM_SIM900
//#define TINY_GSM_MODEM_M590
//#define TINY_GSM_MODEM_A6

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
#define BLYNK_HEARTBEAT 3000

#include <TinyGsmClient.h>
//#include <BlynkSimpleSIM800.h>
//#include <myBlynkSimpleTinyGSM.h>
#include <BlynkSimpleTinyGSM.h>



// Your GPRS credentials
// Leave empty, if missing user or pass
#define APN_NAME "internet"
#define APN_USER " "
#define APN_PSWD " "



// Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(13, 15); // RX, TX  ESP8266
SoftwareSerial SerialAT(2, 3); // RX, TX for NANO

TinyGsm modem(SerialAT);


//*******************************************************
// Code itself
#include "Setting.h"
#include "Blynk.h"

char auth[] = "F4QAN02w4XKgnJmc4XRwon3xxxxxxxx";

#include "DHT.h"
#define DHTPIN 11 // The pin you've connected Data from the DHT to your Arduino
#define DHTTYPE DHT11   // DHT11 or DHT22
DHT dht(DHTPIN, DHTTYPE, 40);//

bool blynkReady = false;



void setup()
{
  SIMreset();
  // setup
  pinMode(PIN_Relay7, OUTPUT);
  digitalWrite(PIN_Relay7, LOW); // set LED OFF
  pinMode(PIN_Relay8, OUTPUT);
  digitalWrite(PIN_Relay8, LOW); // set LED OFF
  pinMode(PIN_Relay5, OUTPUT);
  digitalWrite(PIN_Relay5, HIGH); // set OFF
  pinMode(PIN_Relay6, OUTPUT);
  digitalWrite(PIN_Relay6, HIGH); // set OFF
  pinMode(pinACsensor, INPUT);    // setup AC
  digitalWrite(pinACsensor, HIGH);

  delay(500);
  // Debug console
  // Serial.begin(9600);
  // Serial.println("Serial.begin");
  dht.begin();
  delay(100);

  gsmConnection(); // checking if modem has GPRS internet connection, and if so connecting Blynk 

  // Setup timer

  timer.setInterval(60000L, reconnectBlynk);  // reconnecting Blynk if it is safe) i.e. modem has GPRS internet connection
  
  timer.setInterval(30000L, neverBlynk);  // manage relays' schedule if Blynk not connected or even it had no connection at all 
  delay(100);
  timer.setInterval(59000L, activetoday);  // refresh Time Input from app 
  delay(100);
  timer.setInterval(1000L, resetRelay5);  // 
  delay(100);
  timer.setInterval(1000L, resetRelay6);
  delay(100);
  timer.setInterval(3000L, sendDHT);
  delay(100);
  timer.setInterval(2000L, sendSmoothVoltage);



  // Uncomment one by one to send label names to Blynk app, otherwise make by hand

  // Blynk.setProperty(VPIN_LabelValu, "label", "System codition");// Setting widget name
  // Blynk.setProperty(VPIN_TimeInput, "label", "Off/On Timer");// Setting widget name
  //  Blynk.setProperty(VPIN_Button_Manual_Auto, "label", "Operational mode");// Setting widget name
  //  Blynk.setProperty(VPIN_Button_Manual_Auto, "onLabel", "Manual");// Setting widget name
  //   Blynk.setProperty(VPIN_Button_Manual_Auto, "offLabel", "Auto");// Setting widget name
  //
  //  Blynk.setProperty(VPIN_Button_On_Off_Relay5, "label", "Modem power supply");// Setting widget name
  //  Blynk.setProperty(VPIN_Button_On_Off_Relay5, "onLabel", "Modem power - On");// Setting widget name
  //  Blynk.setProperty(VPIN_Button_On_Off_Relay5, "offLabel", "Modem power- Off");// Setting widget name

  //  Blynk.setProperty(VPIN_Button_On_Off_Relay6, "label", "Router power supply");// Setting widget name
  //  Blynk.setProperty(VPIN_Button_On_Off_Relay6, "onLabel", "Router power - On");// Setting widget name
  //  Blynk.setProperty(VPIN_Button_On_Off_Relay6, "offLabel", "Router power - Off");// Setting widget name
}


void SIMreset(void)
{
  digitalWrite(4, HIGH);
  delay(500);
  digitalWrite(4, LOW);
  delay(5000);
  modem.restart();
}

void gsmConnection()// modem preparation for blynk and blynk begin
{
  SIMreset();
  blynkReady = false;// assume modem not yet ready for Blynk
  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);
  // Serial.println("Initializing modem.....");
  // Serial.print("modem.getSimStatus() = ");
  // Serial.println(modem.getSimStatus());
  if (!modem.getSimStatus()) {
    //   Serial.println("Failed to connect SIM");
  }
  else
  {
    //   Serial.println("SIM - OK");

    if (!modem.waitForNetwork())
    {
      //  Serial.println("Failed to connect to network");
    }
    else
    {
      // RegStatus registration = modemGSM.getRegistrationStatus();
      // Serial.println("Registration: [" + GSMRegistrationStatus(registration) + "]");
      //  Serial.println("Modem network OK");
      delay(500);
      bool stateGPRS = modem.gprsConnect(APN_NAME, APN_USER, APN_PSWD);
      //bool stateGPRS = modem.isGprsConnected();
      // Serial.print("stateGPRS = ");
      // Serial.println(stateGPRS);
      if (stateGPRS) {
        Blynk.begin(auth, modem, APN_NAME, APN_USER, APN_PSWD);
        blynkReady = true;
        rtc.begin();
      }
    }
  }
  // Serial.print("blynkReady = ");
  // Serial.println(blynkReady);
}




void reconnectBlynk() {
  // Serial.println("reconnectBlynk");
  if (blynkReady) {
    if (!Blynk.connected()) {
      if (Blynk.connect()) {
        //   Serial.println("Reconnected");
      } else {
        //   Serial.println("Not reconnected, will run gsmConnection()");
        gsmConnection();
      }
    }
  } else {
    gsmConnection();
  }
}//reconnectBlynk()




void sendSmoothVoltage()
{
  // aValueAverage = (aValueAverage * (averageFactor - 1) + aValue) / averageFactor;
  // this is formula to find average with each new value along last averageFactor values

  Blynk.virtualWrite(V10, smokeValueAverage = analogRead(0)); //Smoke
  // Blynk.virtualWrite(V10, smokeValueAverage = (smokeValueAverage * (averageFactor - 1) + analogRead(0))/averageFactor); //Smoke
  Blynk.virtualWrite(V11, (1023 - analogRead(1))); //Flame
  // Blynk.virtualWrite(V11, flameValueAverage = (flameValueAverage * (averageFactor - 1) + (1023 - analogRead(1)))/averageFactor); //Smoke

  if (digitalRead(pinACsensor)) {
    coef = 0.923; // My board has 2 power sources from Mains and from Batteries.
  } else {
    coef = 0.937;
  }
  // they give not exactly 5 volts, for we use correction coefficient for each reference voltage
  Blynk.virtualWrite(V12, bigVoltageValueAverage = (bigVoltageValueAverage * (averageFactor - 1) + analogRead(2) * 0.06598 * coef) / averageFactor); //Smoke
  // Blynk.virtualWrite(V12, analogRead(2)*0.06598); //big DC Max.65V
  Blynk.virtualWrite(V13, solarVoltageValueAverage = (solarVoltageValueAverage * (averageFactor - 1) + analogRead(3) * 0.03544 * coef) / averageFactor);
  // Blynk.virtualWrite(V13, analogRead(3)*0.03544); //Solar Max. 36V
  Blynk.virtualWrite(V14, acu1VoltageValueAverage = (acu1VoltageValueAverage * (averageFactor - 1) + analogRead(4) * 0.0205 * coef) / averageFactor); //Smoke
  //  Blynk.virtualWrite(V14, analogRead(4)*0.0205);  //Battery1 Max. 20.9V
  Blynk.virtualWrite(V15, acu2VoltageValueAverage = (acu2VoltageValueAverage * (averageFactor - 1) + analogRead(5) * 0.0205 * coef) / averageFactor); //Smoke
  // Blynk.virtualWrite(V15, analogRead(5)*0.0205);  // Battery2 main Max. 20.9V
  Blynk.virtualWrite(V16, optoModemVoltageValueAverage = (optoModemVoltageValueAverage * (averageFactor - 1) + analogRead(6) * 0.0205 * coef) / averageFactor); //Smoke
  //  Blynk.virtualWrite(V16, analogRead(6)*0.0205);  //12 Max. 20.9V
  Blynk.virtualWrite(V17, wifiRouterVoltageValueAverage = (wifiRouterVoltageValueAverage * (averageFactor - 1) + analogRead(7) * 0.0205 * coef) / averageFactor); //Smoke
  //  Blynk.virtualWrite(V17, analogRead(7)*0.0205); //48 Max. 20.9V
  Blynk.virtualWrite(V23, (255 - digitalRead(pinACsensor) * 255)); // send inverted ACsensor value to led widget
}


void sendDHT()
{
  //Read the Temp and Humidity from DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h)) {
    // Serial.println("Failed to read hum");
  } else {
    // Blynk.virtualWrite(V21, h);
    Blynk.virtualWrite(V21, humValueAverage = (humValueAverage * (averageFactor - 1) + h) / averageFactor); //Smoke
  }
  if (isnan(t)) {
    // dht.begin();
    //Serial.println("Failed to read temp");
  } else {
    // Blynk.virtualWrite(V22, t);
    Blynk.virtualWrite(V22, tempValueAverage = (tempValueAverage * (averageFactor - 1) + t) / averageFactor); //Smoke
  }
}


void loop()
{
  if (blynkReady)  {// let Blynk run only of modem has internet connection
    Blynk.run();
  }
  timer.run();
}
