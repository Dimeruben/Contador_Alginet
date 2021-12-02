
#include <SPI.h>                                                              // Librería para la comunicación SPI
#include <nRF24L01.h>                                                         // Librerías para el funcionamiento del módulo NRF24L01
#include <RF24.h>

#include <EEPROM.h>


#define CE 9                                                                  // Declaramos los pines de control del módulo NRF24L01
#define CSN 10


RF24 radio(CE, CSN);                                                          // Se crea el objeto tipo RF24


const uint64_t canal[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};                   // Se declaran los canales (64 bits en hexadecimal) para transmisión RF

//#define  factorK  6.6
#define  ISR_RF 3                                                             // Interrupción modulo RF --> Indica que hay algo para leer
#define  GPIO_Pin  2                                                          // Pin del sensor de flujo, donde se configura la interrupción
#define  releAbrir 5                                                          // Elegimos Pin donde se conectara el rele de Apertura
#define  releCerrar 6                                                         // Elegimos Pin donde se conectara el rele de Cierre

//Variables auxiliares
float factorK = 6.6;                                                          // 6.6 pulsos en un segundo equivale a 1 litro por minuto
unsigned long Lap;                                                            //Marcador para inicio de Lapso
unsigned long Time;                                                           //MArcador de tiempo para calculo de caudal instantaneo
unsigned long Time24h = 0;                                                    //Marcador de tiempo para reinicio del consumo acumulado (cada 24h)
unsigned long Time5h = 0;                                                     //Marcador de tiempo para reinicio del consumo ininterrumpido (cada 5h)
unsigned long Hora = 0;
int contador = 0;                                                             //Pulsos recibidos por el caudalimetro. Para función medirCaudal()
unsigned long cont24hdia = 0;                                                 //para el calculo diario a las 00:00 y mandar a Ubidots
unsigned long cont24h = 0;                                                    //Pulsos recibidos por el caudalimetro. Para función comprobarCont24h()
unsigned long cont1h = 0;                                                               //Pulsos recibidos por el caudalimetro. Para función comprobarCont24h()
unsigned long cont24hmax = 1980000;                                           // 396 pulsos para un litro 1980000 para 5000
unsigned long LimManual = 0;                                                  // Limite manual de agua (por ejemplo para llenar piscina)
float flow_Lmin = 0;                                                          //Caudal instantaneo en L/min
float frecuencia = 0;
float string;
boolean ValvulaAbierta = true;
boolean isLimManual = false;
int Litros = 0;
int msg;
volatile int count = 0;
int pCount = 0;
float data[2];
String mensaje;
String datoRecibido ;


void ISRCounter() {                                                          //Interrupción que gestiona la señal del caudalímetro
  contador++;
  cont24h++;
  cont1h++;
  cont24hdia++;
}



//  https://nrf24.github.io/RF24/examples_2InterruptConfigure_2InterruptConfigure_8ino-example.html#_a0


void setup() {

  Serial.begin(115200);

  digitalWrite(releAbrir , HIGH);
  digitalWrite(releCerrar , HIGH);

  pinMode(GPIO_Pin, INPUT);
  pinMode(ISR_RF, INPUT);
  attachInterrupt(digitalPinToInterrupt(GPIO_Pin), ISRCounter, RISING); //interrrupcion del sensor de caudal
  attachInterrupt(digitalPinToInterrupt(ISR_RF), ISR_RF_function, FALLING); //interrrupcion de la radio
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(releAbrir, OUTPUT);
  pinMode(releCerrar, OUTPUT);

  ValvulaAbierta = EEPROM.read(0);
  Serial.println("EEPROM estado valvula");
  Serial.println(ValvulaAbierta);


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

  cont1h = 0;
  cont24h = 0;
  contador = 0;

}
void ISR_RF_function() {
  //Interrupción que gestiona la señal del caudalímetro
  Serial.println("Estoy dentro de la interrupción");
  count++;

}
void loop() {

  medirCaudal();
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

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void comprobarUbidots() {

  if (millis() - Hora >= 3600000) {
    Serial.println ("mandando consumo en una hora");

    mensaje = "H";
    mensaje.concat(cont1h / factorK / 60);

    cont1h = 0;
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
    cerrarValvula();
  }

}

void comprobarCont24h() {                                             //Comprueba si ha habido un consumo excesivo en las últimas 24h

  if (cont24h > cont24hmax) {
    if(isLimManual==false){
      cerrarValvula();
      cont24h = 0;
      notificaRF("Cons Dia Exced");
    }
  }

  if(cont24h > LimManual && isLimManual==true){
      cerrarValvula();
      cont24h = 0;
      notificaRF("Cons LimManual Exced");
      isLimManual=false;
  }

  
  if (millis() - Time24h > 86400000  || millis() < Time24h) {          //cada 86400000ms =1000*60*60*24 (24h) se reinicia el contador de consumo a 0
    cont24h = 0;
    Time24h = millis();
    if(isLimManual==true) {
      isLimManual=false;
    }

  }
}

void abrirValvula() {                                                 //Rutina que abre la válvula principal
  Serial.println("Abriendo Valvula");
  notificaRF("Abriendo Valvula");
  ValvulaAbierta = true;
  EEPROM.write(0, true);

  digitalWrite(releAbrir , LOW);
  delay(25000);
  digitalWrite(releAbrir , HIGH);
}

void cerrarValvula() {                                                //Rutina que cierra la válvula principal
  Serial.println("Cerrando Valvula");
  notificaRF("Cerrando Valvula");

  ValvulaAbierta = false;
  EEPROM.write(0, false);

  digitalWrite(releCerrar , LOW);
  delay(25000);
  digitalWrite(releCerrar , HIGH);
}

float medirCaudal() {                                                 //Realiza la medición de caudal instánea

  contador = 0;
  Time = millis();

  while (millis() - Time <= 1000) {}
  Serial.print(contador);
  Serial.print(contador);
  flow_Lmin = contador / factorK;

  Serial.print("Caudal = ");
  Serial.print(flow_Lmin);
  Serial.println(" L/min");

  return (flow_Lmin);
}

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
    cerrarValvula();
  }
  else if (datoRecibido == "Abrir") {
    abrirValvula();
  }
  else if (datoRecibido == "/caudal") {
    mensaje.concat(flow_Lmin);
    notificaRF (mensaje);
  }

  else if (datoRecibido == "/consumo") {

    mensaje.concat(cont24h / factorK / 60);
    notificaRF (mensaje);
  }

  else if (datoRecibido == "/initlapso") {
    Lap = cont24h;
    notificaRF ("Iniciando lapso");
  }

  else if (datoRecibido == "/finlapso") {
    notificaRF ("Agua consumida");
    //delay(2000);
    mensaje.concat((cont24h - Lap) / 396);
    notificaRF(mensaje);
  }

  else if (datoRecibido == "/electrovalvula") {
    if (ValvulaAbierta == true) {
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
    mensaje.concat(cont24hdia / factorK / 60);
    notificaRF (mensaje);
    cont24hdia = 0;
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
      
      cont24h = 0;                                                            
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