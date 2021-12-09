#pragma once

class CommunicationService
{
public:
    void Run()
    {
        if (datoRecibido =! "NoData")
        {
            mensaje = "";             //Necesario para que la funcion concat funcione correcamente
            switch (datoRecibido)
            {
                case "Cerrar":
                    valvula.cerrarValvula();
                    rf->notificaRF("Cerrando Valvula");
                    break;

                case "Abrir":
                    valvula.abrirValvula();
                    rf->notificaRF("Abriendo Valvula");
                    break;

                case "/caudal":
                    mensaje.concat(flow_Lmin);
                    rf->notificaRF (mensaje);
                    break;  

                case "/consumo":
                    mensaje.concat(contador.getCons24h() );
                    rf->notificaRF (mensaje);
                    break;    

                case "/initlapso":
                    Lap = contador.getCons24h () ;
                    rf->notificaRF ("Iniciando lapso");
                    break; 

                case "/finlapso":
                    rf->notificaRF ("Agua consumida");
                    //delay(2000);
                    mensaje.concat((contador.getCons24h() - Lap) );
                    rf->notificaRF(mensaje);
                    break; 

                case "/electrovalvula":
                    if (valvula.valvulaAbierta() == true)   rf->notificaRF ("EV abierta");
                    else                                    rf->notificaRF ("EV cerrada");
                    break; 

                case "Reset":   
                    rf->notificaRF("Me reinicio(Cont)");
                    Serial.println("Me reinicio");
                    resetFunc();  //call reset
                    break;  

                case "Cont24reset":
                    rf->notificaRF ("Consumo del día = ");
                    mensaje="D";
                    mensaje.concat(contador.getCons24hdia());
                    rf->notificaRF (mensaje);
                    contador.reset24hdia();
                    Hora = millis();   
                    break;

                case "/limiteagua":
                    rf->notificaRF ("Introduzca Límite"); 
                    delay(10000);  
                    
                    String msg = rf->comprobarRF();

                    if (msg!="NoData")
                    {
                        LimManual = msg;
                        Time24 = millis ();
                    }
                    else rf->notificaRF("Dato no recibido");

                    break;

                default:
                    rf->notificaRF("No reconocido:");
                    f.notificaRF(datoRecibido);

                break;
            }  
        }
    }

private:

   String datoRecibido;
   String mensaje;
   String msg ;

};


  