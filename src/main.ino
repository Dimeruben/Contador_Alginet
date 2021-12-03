
#include <SPI.h>                                                              // Librería para la comunicación SPI

#include <EEPROM.h>

#include "constants/config.h"
#include "constants/pinnout.h"
#include "constants/enum.h"

#include "components/valvula.hpp"
#include "components/contador.hpp"
#include "components/RF.hpp"
                                                        // Se crea el objeto tipo RF24



//Variables auxiliares
unsigned long Lap;                                                            //Marcador para inicio de Lapso
                                                          //MArcador de tiempo para calculo de caudal instantaneo
unsigned long Time24h = 0;                                                    //Marcador de tiempo para reinicio del consumo acumulado (cada 24h)
unsigned long Time5h = 0;                                                     //Marcador de tiempo para reinicio del consumo ininterrumpido (cada 5h)
unsigned long Hora = 0;
                                                           //Pulsos recibidos por el caudalimetro. Para función medirCaudal()
                                                               //Pulsos recibidos por el caudalimetro. Para función comprobarCont24h()
unsigned long LimManual = 0;                                                  // Limite manual de agua (por ejemplo para llenar piscina)
float flow_Lmin = 0;                                                          //Caudal instantaneo en L/min
float frecuencia = 0;
float string;
boolean isLimManual = false;
int Litros = 0;
int msg;
volatile int count = 0;
int pCount = 0;
float data[2];
String mensaje;
String datoRecibido ;

ValvulaComponent valvula;
Contador contador;
RF rf;



void(* resetFunc) (void) = 0; //declare reset function @ address 0



void comprobarUbidots() {

  if (millis() - Hora >= 3600000) {
    Serial.println ("mandando consumo en una hora");

    mensaje = "H";
    mensaje.concat(contador.getCont1h() / factorK / 60);

    contador.reset1h();
    Hora = millis();
    rf.notificaRF (mensaje);

  }
//if (millis() - Dia >= 3600000) {
//    Serial.println ("mandando consumo en un dia");
//
//    mensaje = "D";
//    mensaje.concat(cont1d / factorK) / 1440;
//
//    cont1d = 0;
//    Dia = millis();
//    rf.notificaRF (mensaje)
}


void comprobarIninterrumpido() {
  if (flow_Lmin == 0)

    Time5h = millis();

  if (millis() - Time5h > 18000000  || millis() < Time5h) {          //cada 18000000ms =1000*60*60*5 (5h) se reinicia el contador de consumo a 0
    Time5h = millis();
    rf.notificaRF("Ininterrumpido");
    valvula.cerrarValvula();
    rf.notificaRF("Cerrando Valvula");
  }

}

void comprobarCont24h() {                                             //Comprueba si ha habido un consumo excesivo en las últimas 24h

  if (contador.getCont24h() > cont24hmax) {
    if(isLimManual==false){
      valvula.cerrarValvula();
      rf.notificaRF("Cerrando Valvula");
      contador.reset24h();
      rf.notificaRF("Cons Dia Exced");
    }
  }

  if(contador.getCont24h() > LimManual && isLimManual==true){
      valvula.cerrarValvula();
      contador.reset24h();
      rf.notificaRF("Cons LimManual Exced");
      isLimManual=false;
  }

  
  if (millis() - Time24h > 86400000  || millis() < Time24h) {          //cada 86400000ms =1000*60*60*24 (24h) se reinicia el contador de consumo a 0
    contador.reset24h();
    Time24h = millis();
    if(isLimManual==true) {
      isLimManual=false;
    }

  }
}

void setup() {

  Serial.begin(115200);

  contador.Init();
  valvula.Init();
  rf.Init();
  pinMode(ISR_RF, INPUT);
   
}

void loop() {

  contador.medirCaudal();
  comprobarCont24h();
  comprobarIninterrumpido();                                    // Comprueba que no ha habido un flujo ininterrumpido durante 5h
  comprobarUbidots();
  rf.comprobarRF();
  rf.checkFailure();
delay(2000);         
}
