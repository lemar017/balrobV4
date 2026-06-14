#include "ModuleVAR-GLOB.h"

// Web/IHM communication
String KP_IHM;
String KI_IHM;
String KD_IHM;
String TA_IHM;
String joyX_IHM;
String joyY_IHM;
//int moduleJoy = 0;
int joyX = 0;
int joyY = 0;

String erreur;






// Timer tracking
unsigned long tempsActuel = 0;
unsigned long tempsDebut = 0;




// orientation/motion vars
//Quaternion q;
//VectorFloat gravity;
//float ypr[3] = {0};

// PID
double targetAngle = 91;
//double setpoint = 91;
//double input = 0;
//double output = 0;

double KP = 30;
double KI = 5;
double KD = 2;
float TA = 0;


// Filtrage
FiltreType filtreActuel = MOYENNE;
int interval_ms = 10;
int interval_us = 10000;

// Timer
//volatile bool flagMesure = false;

// MPU6050
int16_t accX = 0;
int16_t accY = 0;
int16_t accZ = 0;
int16_t gyroX = 0;
int16_t gyroY = 0;
int16_t gyroZ = 0;

float accX_raw_fl = 0;
float accY_raw_fl = 0;
float accZ_raw_fl = 0;
float gyroX_raw_fl = 0;
float gyroY_raw_fl = 0;
float gyroZ_raw_fl = 0;

float accX_filtered = 0;
float accY_filtered = 0;
float accZ_filtered = 0;
float gyroX_filtered = 0;
float gyroY_filtered = 0;
float gyroZ_filtered = 0;

volatile float myPID = 0;
volatile float motorPID = 0;
volatile float gyroRate = 0;
volatile float P = 0;
volatile float I = 0;
volatile float D = 0;
volatile float accAngle = 0;
volatile float gyroAngle = 0;
volatile float currentAngle = 0;
volatile float prevAngle = 0;
volatile float error = 0;
volatile float errorSum = 0;

