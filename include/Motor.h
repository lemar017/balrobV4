#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <FastAccelStepper.h>

/*
const int DIR_G = 4;
const int STEP_G = 13;
const int DIR_D = 0;
const int STEP_D = 15;
*/
/*
// === PINS ===
#define DIR_G 26
#define STEP_G 25

#define DIR_D 14
#define STEP_D 27


*/

// Variables pour la gestion du temps moteur
//unsigned long startTime = 0;
//const int VITESSE_MAX = 1000;         // Votre consigne max (Hz ou unité PID)


  long speedG = 0;
  long speedD = 0;





#endif