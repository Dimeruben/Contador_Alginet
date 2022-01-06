#pragma once

#include <EEPROM.h>

#include "config.h"
#include "pinnout.h"

class ValvulaComponent 
{
public:
    void Init()
    {
        isValvulaAbierta = EEPROM.read(0);

        Serial.println("valvulaAbierta?");
        Serial.println(isValvulaAbierta);

        pinMode(releAbrir, OUTPUT);
        pinMode(releCerrar, OUTPUT);

        digitalWrite(releAbrir , HIGH);
        digitalWrite(releCerrar , HIGH);

    }
    void abrirValvula()
    {
        Serial.println("Abriendo Valvula");
        
        isValvulaAbierta = true;
        EEPROM.update(0, true);

        digitalWrite(releAbrir , LOW);
        delay(25000);
        digitalWrite(releAbrir , HIGH);

    }
    void cerrarValvula() {                                               //Rutina que cierra la válvula principal
                                                 
        Serial.println("Cerrando Valvula");

        isValvulaAbierta = false;
        EEPROM.write(0, false);

        digitalWrite(releCerrar , LOW);
        delay(25000);
        digitalWrite(releCerrar , HIGH);
    }
    
    bool valvulaAbierta(){
        return isValvulaAbierta;
    }

private:
    bool isValvulaAbierta;              

};
