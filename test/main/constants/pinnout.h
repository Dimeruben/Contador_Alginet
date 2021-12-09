#pragma once

// Declaramos los pines de control del módulo NRF24L01
const int CE=9;      
const int CSN=10;                

// Interrupción modulo RF --> Indica que hay algo para leer
#define  pinInterrupcionRF 3  

// Pin del sensor de flujo, donde se configura la interrupción
#define  pinContador  2 

// Elegimos Pin donde se conectara el rele de Apertura
#define  releAbrir 5                
#define  releCerrar 6               
