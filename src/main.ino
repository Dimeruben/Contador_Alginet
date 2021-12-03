
#include <SPI.h>                                                              // Librería para la comunicación SPI
#include <nRF24L01.h>                                                         // Librerías para el funcionamiento del módulo NRF24L01
#include <RF24.h>
#include <EEPROM.h>

#include "constants/config.h"
#include "constants/pinnout.h"
#include "constants/enum.h"

#include "components/valvula.hpp"
#include "components/contador.hpp"
                                                        // Se crea el objeto tipo RF24

const uint64_t canal[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};                   // Se declaran los canales (64 bits en hexadecimal) para transmisión RF

RF24 radio(CE, CSN);  

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

void ISR_RF_function() {
  //Interrupción que gestiona la señal del caudalímetro
  Serial.println("Estoy dentro de la interrupción");
  count++;

}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void notificaRF( String data) {

  Serial.println("notificando a repetidor");
  char msg_to_Rptr[20];
  data.toCharArray(msg_to_Rptr, 20);

  radio.stopListening();
  if (!radio.write(&msg_to_Rptr, sizeof(msg_to_Rptr))) {
    Serial.println("Mensaje no enviado");
  }
  delay(3000);                                         // Doy tiempo de escritura al emisor A
 
  radio.startListening();
  
  Serial.print("mensaje enviado: ");
  Serial.println(data);
  delay(20);
}

void comprobarUbidots() {

  if (millis() - Hora >= 3600000) {
    Serial.println ("mandando consumo en una hora");

    mensaje = "H";
    mensaje.concat(contador.getCont1h() / factorK / 60);

    contador.reset1h();
    Hora = millis();
    notificaRF (mensaje);

  }
//if (millis() - Dia >= 3600000) {
//    Serial.println ("mandando consumo en un dia");
//
//    mensaje = "D";
//    mensaje.concat(cont1d / factorK) / 1440;
//
//    cont1d = 0;
//    Dia = millis();
//    notificaRF (mensaje)
}


void comprobarIninterrumpido() {
  if (flow_Lmin == 0)

    Time5h = millis();

  if (millis() - Time5h > 18000000  || millis() < Time5h) {          //cada 18000000ms =1000*60*60*5 (5h) se reinicia el contador de consumo a 0
    Time5h = millis();
    notificaRF("Ininterrumpido");
    valvula.cerrarValvula();
    notificaRF("Cerrando Valvula");
  }

}

void comprobarCont24h() {                                             //Comprueba si ha habido un consumo excesivo en las últimas 24h

  if (contador.getCont24h() > cont24hmax) {
    if(isLimManual==false){
      valvula.cerrarValvula();
      notificaRF("Cerrando Valvula");
      contador.reset24h();
      notificaRF("Cons Dia Exced");
    }
  }

  if(contador.getCont24h() > LimManual && isLimManual==true){
      valvula.cerrarValvula();
      contador.reset24h();
      notificaRF("Cons LimManual Exced");
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



int comprobarRF() {
  
  if (radio.available()) { //get data sent from transmit
    char msg_from_rptdr[20];
    radio.read( &msg_from_rptdr, sizeof(msg_from_rptdr)); //read one byte of data and store it in gotByte variable
    
    Serial.print("he recibido este dato: ");
    Serial.println(msg_from_rptdr);
    Serial.println(count);
    delay(1000);

    datoRecibido = String(msg_from_rptdr);

  mensaje = "";

  if (datoRecibido == "Cerrar") {
    valvula.cerrarValvula();
    notificaRF("Cerrando Valvula");
  }
  else if (datoRecibido == "Abrir") {
    valvula.abrirValvula();
    notificaRF("Abriendo Valvula");
  }
  else if (datoRecibido == "/caudal") {
    mensaje.concat(flow_Lmin);
    notificaRF (mensaje);
  }

  else if (datoRecibido == "/consumo") {

    mensaje.concat(contador.getCont24h() / factorK / 60);
    notificaRF (mensaje);
  }

  else if (datoRecibido == "/initlapso") {
    Lap = contador.getCont24h () ;
    notificaRF ("Iniciando lapso");
  }

  else if (datoRecibido == "/finlapso") {
    notificaRF ("Agua consumida");
    //delay(2000);
    mensaje.concat((contador.getCont24h()  - Lap) / 396);
    notificaRF(mensaje);
  }

  else if (datoRecibido == "/electrovalvula") {
    if (valvula.valvulaAbierta() == true) {
      notificaRF ("EV abierta");
    }
    else  notificaRF ("EV cerrada") ;
  }

  else if (datoRecibido == "Reset") {

    //delay(10000);
    notificaRF("Reset");
    Serial.println("Me reinicio");

    resetFunc();  //call reset
    Serial.println("Nunca se lee");
  }

  else if (datoRecibido == "Cont24reset") {             //Cada día a las 00 recibe un comando desde Repetidor que reinicia el consumo

    notificaRF ("Consumo del día = ");
    mensaje="D";
    mensaje.concat(contador.getCont24hdia()  / factorK / 60);
    notificaRF (mensaje);
    contador.reset24hdia();
    Hora = millis();                                    //sincronización de hora para notificación horaria
  }

 else if (datoRecibido ="/limiteagua") {             //Programa un limite de XLitros despues del cual la valvula se cierra (Pensado para llenado de piscina) 

    notificaRF ("Introduzca Límite"); 
    delay(10000);  
    
    if (radio.available()) { //get data sent from transmit
      
      char msg_from_rptdr[20];
      radio.read( &msg_from_rptdr, sizeof(msg_from_rptdr)); //read one byte of data and store it in gotByte variable
      
      Serial.print("he recibido este dato: ");
      Serial.println(msg_from_rptdr);
      Serial.println(count);
      delay(1000);
  
      LimManual = int(msg_from_rptdr)*factorK*60;
      isLimManual=true;                                                       //Bandera para que la valvula no se cierre por otras causas
      
      contador.reset24h();                                                           
      Time24h = millis();
    }
    else notificaRF ("Dato no recibido"); 

  }

  else {
    notificaRF("No reconocido:");
    notificaRF(datoRecibido);
  }

  pCount = count;
  
  }
  
}

void setup() {

  Serial.begin(115200);

  valvula.Init();
  pinMode(ISR_RF, INPUT);
  attachInterrupt(digitalPinToInterrupt(ISR_RF), ISR_RF_function, FALLING); //interrrupcion de la radio
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //Inicialización del modulo RF
  radio.begin(); //Start the nRF24 module
  radio.setAutoAck(1); // Ensure autoACK is enabled so rec sends ack packet to let you know it got the transmit packet payload
  //radio.enableAckPayload(); //allows you to include payload on ack packet
  radio.maskIRQ(1, 1, 0); //mask all IRQ triggers except for receive (1 is mask, 0 is no mask)
  radio.setPALevel(RF24_PA_LOW); //Set power level to low, won't work well at higher levels (interfer with receiver)
  radio.startListening(); // Start listening for message
  radio.setDataRate(RF24_250KBPS);  //Minima velocidad para aumentar rango
  radio.setChannel(100);            // Configuracion del canal de comunicacion
  radio.openWritingPipe(canal[0]); // Abro el canal "0" para escribir
  radio.openReadingPipe(1, canal[1]); // Abro el canal "1" para leer

}

void loop() {

  contador.medirCaudal();
  comprobarCont24h();
  comprobarIninterrumpido();                                    // Comprueba que no ha habido un flujo ininterrumpido durante 5h
  comprobarUbidots();
  comprobarRF();
    
  if(radio.failureDetected){
      radio.begin();                       // Attempt to re-configure the radio with defaults
      radio.failureDetected = 0;           // Reset the detection value
      radio.openWritingPipe(canal[0]); // Abro el canal "0" para escribir
      radio.openReadingPipe(1, canal[1]); // Abro el canal "1" para leer
      Serial.println("Error con RF24");
        }
delay(2000);         
}
