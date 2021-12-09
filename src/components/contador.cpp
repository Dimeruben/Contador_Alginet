#pragma once

#include "./constants/config.h"
#include "./constants/pinnout.h"

#include "contador.h"

Contador *pointerToClass;

        int cont = 0;
        unsigned long cont24hdia = 0;                                                 //para el calculo diario a las 00:00 y mandar a Ubidots
        unsigned long cont24h = 0;                                                    //Pulsos recibidos por el caudalimetro. Para función comprobarCont24h()
        unsigned long cont1h = 0;


void Contador::Init()
{
    
    pointerToClass = this;
    pinMode(pinContador, INPUT);
    attachInterrupt(digitalPinToInterrupt(pinContador), outsideInterruptHandler   , RISING);

}

float Contador::medirCaudal()                    //Realiza la medición de caudal instánea
{                                                 

    unsigned long Time = millis();

    while (millis() - Time <= 1000) {}

    float flow_Lmin = cont / factorK;

    Serial.print("Caudal = ");
    Serial.print(flow_Lmin);
    Serial.println(" L/min");

    return (flow_Lmin);
}

void Contador::reset1h()      {cont1h=0;}
void Contador::reset24h()     {cont24h=0;}
void Contador::reset24hdia()  {cont24hdia=0;}

unsigned long Contador::getCons24hdia()  {return (cont24hdia/factorK/60);}
unsigned long Contador::getCons24h()     {return (cont24h   /factorK/60);}
unsigned long Contador::getCons1h()      {return (cont1h    /factorK/60);}


static void outsideInterruptHandler(void) { // define global handler

    pointerToClass->ISRCounter(); // calls class member handler
}

void Contador::ISRCounter()
{
    cont++;
    cont24h++;
    cont1h++;
    cont24hdia++;       
}


     







