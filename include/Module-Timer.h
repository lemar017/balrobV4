#ifndef MODULETIMER_H
#define MODULETIMER_H


#include "Ticker.h" //TIMER
Ticker timerLED; // Clignotement de la LED toutes les secondes
Ticker timerPID;
volatile bool ledState = false;
volatile bool pidFlag = false;
//---------------------TIMER------------------------------------------
int sampleTimeMs=5;
double sampleTime=0.005;//modif

// ======================== TIMER ================================

volatile bool flagMesure = false;

#endif