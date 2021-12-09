#pragma once


#include "./components/valvula.hpp"
#include "./components/RF.hpp"
#include "./components/contador.h"

    
class Alarms
{
public:

    void checkAlarms()
    {
        comprobarIninterrumpido();
        comprobarCont24h();

    }

    void setLimManual(unsigned long Limite)
    {
        LimManual = Limite;
    }

    void setIsManual()
    {
        isLimManual=true;
    }

    void setInicio24h(unsigned long _Time24h)
    {
        Time24h = _Time24h;
    }

  
private:

    ValvulaComponent* valvula;
    Contador* contador;
    RF* rf;

    unsigned long LimManual = 0; 
    boolean isLimManual = false;

    unsigned long Time24h = 0;                                                    //Marcador de tiempo para reinicio del consumo acumulado (cada 24h)
    unsigned long Time5h = 0;                                                     //Marcador de tiempo para reinicio del consumo ininterrumpido (cada 5h)

    const long unsigned int i3h  = 18000000;
    const long unsigned int i24h = 86400000;

    void comprobarIninterrumpido() {
    if (contador->medirCaudal() == 0)
    {
        Time5h = millis(); 
    }

    if (millis() - Time5h > i3h  || millis() < Time5h) {          //cada 18000000ms =1000*60*60*5 (5h) se reinicia el contador de consumo a 0
        Time5h = millis();
        rf->notificaRF("Ininterrumpido");
        valvula->cerrarValvula();
    }
    }

    void comprobarCont24h() {                                             //Comprueba si ha habido un consumo excesivo en las últimas 24h

    if (contador->getCons24h() > cont24hmax) {
        if(isLimManual==false){
        valvula->cerrarValvula();
        contador->reset24h();
        rf->notificaRF("Cons Dia Exced");
        }
    }

    if(contador->getCons24h() > LimManual && isLimManual==true)
    {
        valvula->cerrarValvula();
        contador->reset24h();
        rf->notificaRF("Cons LimManual Exced");
        isLimManual=false;
    }

    
    if (millis() - Time24h > i24h  || millis() < Time24h) 
    {      
        contador->reset24h();
        Time24h = millis();
        if(isLimManual==true) 
        {
        isLimManual=false;
        }

    }
    }

};


  
