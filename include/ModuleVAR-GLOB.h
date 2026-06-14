#ifndef MODULEVARGLOB_H
#define MODULEVARGLOB_H

#include <Arduino.h>
#include <MPU6050.h>
#include <helper_3dmath.h>
//#include <PID_v1.h>
#include <esp_timer.h>


//*******************************************************************************************
//                                   VARIABLE DECLARATIONS
//*******************************************************************************************

// Web/IHM communication
extern String KP_IHM, KI_IHM, KD_IHM, TA_IHM; // Proportionnel,Integral,Dérivée, Target Angle(Consigne)
extern String joyX_IHM, joyY_IHM;             // Valeurs X,Y du joystick
extern int  joyX, joyY;
extern String erreur;

//-------------------------------------------------------------------
static const int PIN_LED_OK = 12;


//---------------------TIMER------------------------------------------
extern unsigned long tempsActuel;
extern unsigned long tempsDebut;


//-----------------orientation/motion vars------------------------------
//extern Quaternion q;        // [w, x, y, z] quaternion container
//extern VectorFloat gravity; // [x, y, z] gravity vector
//extern float ypr[3];

//-------------------Init Kp,Kd,KI-------------------------------------
// PID
extern double targetAngle;


//--------------Valeurs d'intitialisation du correcteur PID----------
// Proportionnel,Integral,Dérivée, Target Angle(Consigne)
extern double KP;
extern double KI;
extern double KD;
extern float TA;


// =============================== FILTRAGE =======================
enum FiltreType
{
    MOYENNE,
    EMA,
    COMPLEMENTAIRE,
    KALMAN
};
extern FiltreType filtreActuel;
extern int interval_ms;
extern int interval_us;

// ======================== TIMER ================================


//----------------------MPU6050-------------------------------------
extern int16_t accX, accY, accZ, gyroX, gyroY, gyroZ;
extern float accX_raw_fl, accY_raw_fl, accZ_raw_fl, gyroX_raw_fl, gyroY_raw_fl, gyroZ_raw_fl;
extern float accX_filtered, accY_filtered, accZ_filtered, gyroX_filtered, gyroY_filtered, gyroZ_filtered;

//----------------------Conversion au systeme international--------
static const float ACC_SENS = 16384.0;
static const float GYRO_SENS = 131.0;
static const float G = 9.80665;
static const float DEGRE_TO_RADIAN = 3.14159265 / 180.0;

extern volatile float myPID,motorPID, gyroRate, P, I, D;
extern volatile float accAngle, gyroAngle, currentAngle, prevAngle, error, errorSum;



#endif
