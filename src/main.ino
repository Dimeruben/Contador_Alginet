#include "components/valvula.hpp"
#include "components/contador.h"
#include "components/RF.hpp"

#include "services/comunication.hpp"
#include "services/alarms.hpp"

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

