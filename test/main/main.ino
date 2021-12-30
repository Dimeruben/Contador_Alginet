#include "valvula.hpp"
#include "contador.h"
#include "RF.hpp"

#include "comunication.hpp"
#include "alarms.hpp"

ValvulaComponent valvula;
Contador contador;
RF rf;
CommunicationService coms;
Alarms alarm;

void setup() {

  Serial.begin(115200);

  contador.Init();
  valvula.Init();
  rf.Init();
   
}

void loop() {

  contador.medirCaudal();
  alarm.checkAlarms();
  coms.checkComs();
  rf.checkFailure();

  delay(100);
     
}

