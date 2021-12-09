#pragma once

#include <nRF24L01.h>                                                         // Librerías para el funcionamiento del módulo NRF24L01
#include <RF24.h>
#include <SPI.h>

#include "./constants/config.h"
#include "./constants/pinnout.h"

class RF
{
public:

    void Init()
    {
    

         //Inicialización del modulo RF
        radio->begin(CE,CSN); //Start the nRF24 module
        radio->setAutoAck(1); // Ensure autoACK is enabled so rec sends ack packet to let you know it got the transmit packet payload
        //radio->enableAckPayload(); //allows you to include payload on ack packet
        radio->maskIRQ(1, 1, 0); //mask all IRQ triggers except for receive (1 is mask, 0 is no mask)
        radio->setPALevel(RF24_PA_LOW); //Set power level to low, won't work well at higher levels (interfer with receiver)
        radio->startListening(); // Start listening for message
        radio->setDataRate(RF24_250KBPS);  //Minima velocidad para aumentar rango
        radio->setChannel(100);            // Configuracion del canal de comunicacion
        radio->openWritingPipe(canal[0]); // Abro el canal "0" para escribir
        radio->openReadingPipe(1, canal[1]); // Abro el canal "1" para leer

        attachInterrupt(digitalPinToInterrupt(pinInterrupcionRF), RF::getISR_RF_function, RISING);
    
    }
    
    void notificaRF( String data) 
    {
        Serial.println("notificando a repetidor");
        char msg_to_Rptr[20];
        data.toCharArray(msg_to_Rptr, 20);

        radio->stopListening();
        if (!radio->write(&msg_to_Rptr, sizeof(msg_to_Rptr))) {
            Serial.println("Mensaje no enviado");
        }
        delay(3000);                                         // Doy tiempo de escritura al emisor A
        
        radio->startListening();
        
        Serial.print("mensaje enviado: ");
        Serial.println(data);
        delay(20);
    }

    String comprobarRF() 
    {
  
        if (radio->available()) { //get data sent from transmit
            char msg_from_rptdr[20];
            radio->read( &msg_from_rptdr, sizeof(msg_from_rptdr)); //read one byte of data and store it in gotByte variable
            
            Serial.print("he recibido este dato: ");
            Serial.println(msg_from_rptdr);

            delay(1000);

            datoRecibido = String(msg_from_rptdr);

            return(datoRecibido);
        }
        
        return ("NoData");
    }

    void checkFailure()
    {
        if(radio->failureDetected){
        radio->begin();                       // Attempt to re-configure the radio with defaults
        radio->openWritingPipe(canal[0]); // Abro el canal "0" para escribir
        radio->openReadingPipe(1, canal[1]); // Abro el canal "1" para leer
        Serial.println("Error con RF24");
        
        radio->failureDetected = 0;           // Reset the detection value

        }
    }

    void ISR_RF_function() 
    {
        Serial.println("Mensaje entrante");

    }

private:

    static RF* anchor;
    RF24* radio;

    String datoRecibido ;
    
    const uint64_t canal[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};                   // Se declaran los canales (64 bits en hexadecimal) para transmisión RF



     //https://stackoverflow.com/questions/41443720/how-to-create-an-isr-in-an-arduino-class
     //https://www.onetransistor.eu/2019/05/arduino-class-interrupts-and-callbacks.html
    static void getISR_RF_function(void* RF)
    {
        anchor-> ISR_RF_function();
    } 
};