#pragma once

#ifndef CONTADOR_H
#define CONTADOR_H

#include "Arduino.h" // must be included

class Contador
{
    public:

        void Init();
        
        float medirCaudal();

        void reset1h() ;
        void reset24h() ;
        void reset24hdia();

        unsigned long getCons24hdia()  ;
        unsigned long getCons24h()     ;
        unsigned long getCons1h()      ;

        void ISRCounter();

    private:
    

}; 

#endif



