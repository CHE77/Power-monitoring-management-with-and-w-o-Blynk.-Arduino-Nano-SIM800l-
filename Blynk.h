#include <TimeLib.h>
#include <WidgetRTC.h>

BlynkTimer timer;

WidgetRTC rtc;




//*****************************************************
//  Code itself

// Синхронизация при конекте
BLYNK_CONNECTED() {
  if (isFirstConnect) {
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay5, Relay5Status);
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay6, Relay6Status);
    Blynk.virtualWrite(VPIN_Button_Manual_Auto, Manual_Status);
    Blynk.syncAll();
    //Blynk.notify("Connected");       //
    isFirstConnect = false;

  }
}

// check if schedule updated
void activetoday() {
  if (year() != 1970) {
    Blynk.syncVirtual(VPIN_TimeInput); // синхронизируем виджет timeinput
    //Serial.println("");Serial.println("activetoday");
  }
}

BLYNK_WRITE(VPIN_Button_Manual_Auto)  // Manual_Status/Auto selection
{
  if (param.asInt() == 1) {
    Manual_Status = 1;           // Ручной режим включить
  } else {
    Manual_Status = 0;           // Ручной режим выключить, включить автоматический режим
    timer.setTimeout(1000, activetoday);
  }
  // Serial.println("");Serial.print("VPIN_Button_Manual_Auto ");Serial.println(Manual_Status);
}


BLYNK_WRITE(VPIN_Button_On_Off_Relay5)  // ON-OFF Manual_Status
{
  if (param.asInt() == 1) { // 
    Relay5Status = true; // ON 
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay5, Relay5Status);// may be this is not necessary
  } else { // 
    Relay5Status = false; // OFF 
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay5, Relay5Status);// may be this is not necessary
  }
  //  Serial.println("");Serial.print("VPIN_Button_On_Off_Relay5 RelayStatus ");Serial.println(Relay5Status);
}//BLYNK_WRITE(VPIN_Button_On_Off_Relay5)

BLYNK_WRITE(VPIN_Button_On_Off_Relay6)  // ON-OFF Manual_Status
{
  if (param.asInt() == 1) { // 
    Relay6Status = true; // ON 
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay6, Relay6Status);// may be this is not necessary
  } else { // 
    Relay6Status = false; // OFF 
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay6, Relay6Status);// may be this is not necessary
  }
  // Serial.println("");Serial.print("VPIN_Button_On_Off_Relay6 RelayStatus ");Serial.println(Relay6Status);
}//BLYNK_WRITE(VPIN_Button_On_Off_Relay5)



BLYNK_WRITE(VPIN_TimeInput)// Schedule Time Input and managing relay acc. schedule 
{
  TimeInputParam t(param);

  int32_t H = int32_t(hour()) * 3600;
  int32_t M = minute() * 60;
  int32_t S = second();

  nowseconds = H + M + S;

  clocksDiff = nowseconds - millis() / 1000;// While we getting fresh time update, binding to millis clock.
  // later if no more Blynk, we will use millis with clock correction
  //  Serial.print("clocksDiff = ");
  //  Serial.println(clocksDiff);
  startsecondswd  = param[0].asLong();
  stopsecondswd   = param[1].asLong();

  //      Serial.print("nowseconds ");Serial.println(nowseconds);  // used for debugging
  //      Serial.print("startsecondswd ");Serial.println(startsecondswd);  // used for debugging
  //      Serial.print("stopsecondswd ");Serial.println(stopsecondswd);  // used for debugging

  if (Manual_Status == 0)
  {
    int dayadjustment = -1;
    if (weekday() == 1) {
      dayadjustment =  6; // Assigning number foe Sunday. Sunderat RTC is 1, and at Blynk is 7
    }
    // checking if day of the week is on schedule
    if (t.isWeekdaySelected(weekday() + dayadjustment)) {
      //  Serial.print("nowseconds = ");
      //  Serial.println(nowseconds);
      //  Serial.print("startsecondswd = ");
      //  Serial.println(startsecondswd);
      //  Serial.print("stopsecondswd = ");
      //  Serial.println(stopsecondswd);

      if (nowseconds >= startsecondswd && nowseconds <= stopsecondswd) {
        Relay5Status = 0; // OFF relay
        Relay6Status = 0; // OFF relay
      }
      else {
        Relay5Status = 1; // at first we switch on Modem
        if (nowseconds - stopsecondswd > 60) {  // and 1 min later we switch on router
          Relay6Status = 1; // 
          //lastAuto56ResetMillis = millis() / 1000;
          //lastAuto56ResetSeconds = nowseconds;
        }
      }
    }
    else {
      Relay5Status = 1;
      Relay6Status = 1;
    }
  }
}//BLYNK_WRITE(VPIN_TimeInput)//


void resetRelay5()
{
  static bool LastRelay5Status = false;

  if (LastRelay5Status != Relay5Status)
  {
    LastRelay5Status = Relay5Status;
    digitalWrite(PIN_Relay5, Relay5Status); // OFF реле
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay5, Relay5Status); //Button в приложении (обратная связь)
    //Serial.println("");Serial.print("Relay5Status ");Serial.println(Relay5Status);
  }
}//resetRelay()


void resetRelay6()
{
  static bool LastRelay6Status = false;

  if (LastRelay6Status != Relay6Status)
  {
    LastRelay6Status = Relay6Status;
    digitalWrite(PIN_Relay6, Relay6Status); // OFF реле
    Blynk.virtualWrite(VPIN_Button_On_Off_Relay6, Relay6Status); //Button в приложении (обратная связь)
    // Serial.println("");Serial.print("Relay6Status ");Serial.println(Relay6Status);
  }
}//resetRelay()



void neverBlynk()// managing schedule for relay if we loose Blynk connection or even if was not at all
{
  // Serial.println("neverBlynkcheck");

  if (!Blynk.connected()) {  //

  int millisToday = (millis() - int(millis()/86400000))/1000;//actualy it ia seconds

    if (clocksDiff + millisToday >= startsecondswd && clocksDiff + millisToday <= stopsecondswd) {
      Relay5Status = 0; // OFF relay
      Relay6Status = 0; // OFF relay
      // Serial.println("relay Reset on millis ");
    }
    else {
      Relay5Status = 1; // at first we switch on Modem
      if (clocksDiff + millisToday - stopsecondswd > 60) { // and 1 min later we switch on router
        Relay6Status = 1; // 
        // startsecondswd = startsecondswd + millis() / 1000;// make schedule for next day
        // stopsecondswd = stopsecondswd + millis() / 1000;// make schedule for next day
       // lastAuto56ResetMillis = millis() / 1000;
      }
    }
  }
}
