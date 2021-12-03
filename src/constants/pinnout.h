#pragma once

// Declaramos los pines de control del módulo NRF24L01
#define CE 9                        
#define CSN 10

// Interrupción modulo RF --> Indica que hay algo para leer
#define  ISR_RF 3  

// Pin del sensor de flujo, donde se configura la interrupción
#define  GPIO_Pin  2                

// Elegimos Pin donde se conectara el rele de Apertura
#define  releAbrir 5                
#define  releCerrar 6               
