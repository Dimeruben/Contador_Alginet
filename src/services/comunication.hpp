#pragma once

#include "../components/valvula.hpp"
#include "../components/RF.hpp"
#include "../components/contador.hpp"
#include "../services/alarms.hpp"


    
class CommunicationService
{
public:



    void checkComs()
    {
        comprobarRF();
        comprobarUbidots();
    }
private:


    ValvulaComponent* valvula;
    Contador* contador;
    RF* rf;
    Alarms* alarm;

    String datoRecibido;
    String mensaje;

    unsigned long ConsInicioLapso;  
    unsigned long Hora = 0;

    const long unsigned int i1h = 3600000;


    void comprobarRF()
    {
        if (datoRecibido =! "NoData")
        {
            mensaje = "";             //Necesario para que la funcion concat funcione correcamente
            
            if (datoRecibido == "Cerrar")
            {
                valvula->cerrarValvula();
            }     
            else if (datoRecibido == "Abrir")
            {
                valvula->abrirValvula();
            }
            else if (datoRecibido == "/caudal")
            {
                mensaje.concat(contador->medirCaudal());
                rf->notificaRF (mensaje);               
            }
            else if (datoRecibido == "/consumo")
            {
                mensaje.concat(contador->getCons24h() );
                rf->notificaRF (mensaje);          
            }
            else if (datoRecibido == "/initlapso")
            {
                ConsInicioLapso= contador->getCons24h () ;
                rf->notificaRF ("Iniciando lapso");        
            }
            else if (datoRecibido == "/finlapso")
            {
                rf->notificaRF ("Agua consumida");
                //delay(2000);
                mensaje.concat((contador->getCons24h() - ConsInicioLapso) );
                rf->notificaRF(mensaje);     
            }
            else if (datoRecibido == "/electrovalvula")
            {
                if (valvula->valvulaAbierta() == true)   rf->notificaRF ("EV abierta");
                else                                    rf->notificaRF ("EV cerrada");
            }
            else if (datoRecibido == "Reset")
            {
                rf->notificaRF("Me reinicio(Cont)");
                Serial.println("Me reinicio");
                resetFunc();  //call reset
            } 
            else if (datoRecibido == "Cont24reset")
            {
                rf->notificaRF ("Consumo del día = ");
                mensaje="D";
                mensaje.concat(contador->getCons24hdia());
                rf->notificaRF (mensaje);
                contador->reset24hdia();
                Hora = millis();   
            } 
            else if (datoRecibido == "/limiteagua")
            {
                rf->notificaRF ("Introduzca Límite"); 
                delay(10000);  
                
                String msg = rf->comprobarRF();

                if (msg!="NoData")
                {
                    unsigned long LimManual = atol(msg.c_str());
                    alarm->setLimManual( LimManual );
                    alarm->setIsManual();
                    alarm->setInicio24h(millis());
                }
                else rf->notificaRF("Dato no recibido");
            } 
            else 
            {
                rf->notificaRF("No reconocido:");
                rf->notificaRF(datoRecibido);
            }
            
        }
    }

    void comprobarUbidots() 
    {

    if (millis() - Hora >= i1h) {
        Serial.println ("Consumo última hora");

        mensaje = "H";
        mensaje.concat(contador->getCons1h());

        contador->reset1h();
        Hora = millis();
        rf->notificaRF (mensaje);

    }

    }

    void(* resetFunc) (void) = 0; //declare reset function @ address 0



};


  