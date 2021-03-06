#pragma once

#include "config.h"
#include "pinnout.h"


class Contador
{
public:

    void Init()
    {
    
        pinMode(pinContador, INPUT);
        attachInterrupt(digitalPinToInterrupt(pinContador), Contador::GetISR , RISING );

    }

    float medirCaudal()                    //Realiza la medición de caudal instánea
    {                                                 

        unsigned long Time = millis();

        while (millis() - Time <= 1000) {}

        float flow_Lmin = cont / factorK;

        Serial.print("Caudal = ");
        Serial.print(flow_Lmin);
        Serial.println(" L/min");

        return (flow_Lmin);
    }

    void reset1h()      {cont1h=0;}
    void reset24h()     {cont24h=0;}
    void reset24hdia()  {cont24hdia=0;}

    unsigned long getCons24hdia()  {return (cont24hdia/factorK/60);}
    unsigned long getCons24h()     {return (cont24h   /factorK/60);}
    unsigned long getCons1h()      {return (cont1h    /factorK/60);}

private:

    static Contador *anchor;

    int cont = 0;
    unsigned long cont24hdia = 0;                                                 //para el calculo diario a las 00:00 y mandar a Ubidots
    unsigned long cont24h = 0;                                                    //Pulsos recibidos por el caudalimetro. Para función comprobarCont24h()
    unsigned long cont1h = 0;

    static void GetISR(){
        anchor -> ISRCounter();
    }

    void ISRCounter()
    {
        cont++;
        cont24h++;
        cont1h++;
        cont24hdia++;      
    }

};
     







