/\*\*

- EXEMPLE COMPLET: Migration pas-à-pas d'une fonction
-
- Ce fichier montre comment migrer une fonction existante
- vers la nouvelle architecture, étape par étape.
-
- EXEMPLE: La fonction calculPID() du main.cpp actuel
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 0: CODE ACTUEL (AVANT REFACTORISATION)
// ═══════════════════════════════════════════════════════════════════════════

/\*
// Dans ModuleVAR-GLOB.h:
extern float accAngle, gyroRate, gyroAngle, currentAngle;
extern float error, errorSum, targetAngle;
extern float P, I, D, motorPID;
extern int joyX, joyY;
extern double KP, KI, KD, TA;

// Dans main.cpp:
float calculPID(float dt, float Y) {
FilteredImuSample filteredSample = {
accX_filtered, accY_filtered, accZ_filtered,
gyroX_filtered, gyroY_filtered, gyroZ_filtered};
JoystickState joystick = {static_cast<float>(joyX), Y};
PidSettings settings = {TA, KP, KI, KD};

motorPID = balanceController.compute(dt, filteredSample, joystick, settings);

const BalanceTelemetry &telemetry = balanceController.telemetry();
accAngle = telemetry.accAngle;
gyroRate = telemetry.gyroRate;
gyroAngle = telemetry.gyroAngle;
currentAngle = telemetry.currentAngle;
targetAngle = telemetry.targetAngle;
error = telemetry.error;
errorSum = telemetry.errorSum;
P = telemetry.p;
I = telemetry.i;
D = telemetry.d;
prevAngle = currentAngle;

return motorPID;
}

PROBLÈMES:
✗ Variables extern éparpillées
✗ État caché et dispersé
✗ Difficile de tester
✗ Difficile d'étendre
✗ Dépendances implicites
\*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 1: CRÉER LA STRUCTURE (RobotState)
// ═══════════════════════════════════════════════════════════════════════════

/\*
// Dans RobotState.h: (déjà créé ✓)

struct PidState {
float error;
float errorSum;
float prevAngle;
float P, I, D;
};

struct RobotState {
struct { float accAngle, gyroRate, gyroAngle, currentAngle; } imu;
struct {
PidParameters params;
PidState state;
float motorOutput;
} pid;
// ... autres champs
};

BÉNÉFICE: État structuré et visible
\*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 2: ENCAPSULER DANS UNE CLASSE (RobotSystem)
// ═══════════════════════════════════════════════════════════════════════════

/\*
// Dans RobotSystem.h: (déjà créé ✓)

class RobotSystem {
private:
RobotState state*;
BalanceController balanceController*;

    void updatePID_() {
        // Logique de PID ici
    }

};

// Dans RobotSystem.cpp: (à implémenter complètement)

void RobotSystem::updatePID\_() {
// Cette méthode remplace calculPID()

    FilteredImuSample filteredSample = {
        state_.imu.filtered.accX, state_.imu.filtered.accY, state_.imu.filtered.accZ,
        state_.imu.filtered.gyroX, state_.imu.filtered.gyroY, state_.imu.filtered.gyroZ
    };

    JoystickState joystickState = {
        static_cast<float>(state_.joystick.current.x),
        static_cast<float>(state_.joystick.current.y)
    };

    PidSettings pidSettings = {
        state_.pid.params.targetAngle,
        state_.pid.params.kp,
        state_.pid.params.ki,
        state_.pid.params.kd
    };

    state_.pid.motorOutput = balanceController_.compute(
        state_.timing.deltaTimeS,
        filteredSample,
        joystickState,
        pidSettings
    );

    // Mettre à jour l'état avec la télémétrie du contrôleur
    const BalanceTelemetry &telemetry = balanceController_.telemetry();
    state_.imu.accAngle = telemetry.accAngle;
    state_.imu.gyroRate = telemetry.gyroRate;
    state_.imu.gyroAngle = telemetry.gyroAngle;
    state_.imu.currentAngle = telemetry.currentAngle;
    state_.pid.state.error = telemetry.error;
    state_.pid.state.errorSum = telemetry.errorSum;
    state_.pid.state.P = telemetry.p;
    state_.pid.state.I = telemetry.i;
    state_.pid.state.D = telemetry.d;
    state_.pid.state.prevAngle = state_.imu.currentAngle;

}

BÉNÉFICE: État encapsulé et cohérent
\*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 3: INTÉGRER DANS LA BOUCLE PRINCIPALE
// ═══════════════════════════════════════════════════════════════════════════

/\*
// AVANT (main.cpp actuel):

void loop() {
unsigned long now = micros();

    if (now - dernierPID >= INTERVALLE_PID) {
        // Lire capteurs
        imuFilter.update(rawSample);
        // Calculer PID
        float dt = (now - dernierPID) / 1000000.0f;
        calculPID(dt, joyY);
        // Envoyer moteurs
        motors.setSpeed(motorLeft, motorRight);
        // ...
        dernierPID = now;
    }

}

// APRÈS (avec les nouvelles classes):

RobotSystem robot;
TimingController timing;

void loop() {
timing.update();
robot.update(); // Contient TOUT: IMU, PID, Moteurs, etc.

    if (timing.shouldDisplayData()) {
        monitor.logData(robot.getState());
    }

}

BÉNÉFICE: Code clair et lisible
\*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 4: ACCÉDER À L'ÉTAT (INTERFACE PUBLIQUE)
// ═══════════════════════════════════════════════════════════════════════════

/\*
// Si vous avez besoin d'accéder aux données pour l'affichage:

AVANT:
Serial.printf("Angle: %.2f, PID: %.2f\n", currentAngle, motorPID);

APRÈS:
const RobotState& state = robot.getState();
Serial.printf("Angle: %.2f, PID: %.2f\n",
state.imu.currentAngle, state.pid.motorOutput);

// Ou via PerformanceMonitor:
monitor.logData(robot.getState());

BÉNÉFICE: Pas de variables extern, état visible
\*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 5: MODIFICATION DES PARAMÈTRES (INTERFACE PUBLIQUE)
// ═══════════════════════════════════════════════════════════════════════════

/\*
// Si l'interface web change les paramètres PID:

AVANT:
void handleUpdatePID(AsyncWebServerRequest\* request) {
KP = request->getParam("kp")->value().toDouble();
KI = request->getParam("ki")->value().toDouble();
KD = request->getParam("kd")->value().toDouble();
TA = request->getParam("ta")->value().toFloat();
// Les variables extern sont directement modifiées
}

APRÈS:
void handleUpdatePID(AsyncWebServerRequest\* request) {
double kp = request->getParam("kp")->value().toDouble();
double ki = request->getParam("ki")->value().toDouble();
double kd = request->getParam("kd")->value().toDouble();
float ta = request->getParam("ta")->value().toFloat();

      robot.setPidParameters(kp, ki, kd, ta);  // Interface claire

}

// Implémentation dans RobotSystem:
void RobotSystem::setPidParameters(double kp, double ki, double kd, float ta) {
// Validation optionnelle
if (kp < 0 || ki < 0 || kd < 0) {
Serial.println("ERREUR: Paramètres négatifs!");
return;
}

    // Mise à jour
    state_.pid.params = {kp, ki, kd, ta};
    Serial.printf("PID: KP=%.2f, KI=%.2f, KD=%.2f, TA=%.2f\n", kp, ki, kd, ta);

}

BÉNÉFICE: Interface claire, validation facile
\*/

// ═══════════════════════════════════════════════════════════════════════════
// RÉSUMÉ VISUEL DE LA MIGRATION
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT:
┌─────────────────────────────────────────┐
│ main.cpp (chaotique) │
├─────────────────────────────────────────┤
│ • 50+ variables extern │
│ • calculPID(dt, Y) │
│ • controlMotors(PID, X, Y) │
│ • 100+ lignes en loop() │
│ • État dispersé partout │
│ • Difficile à déboguer │
└─────────────────────────────────────────┘
│ │
▼ ▼
┌──────────┐ ┌──────────────┐
│ ModuleX │ │ ModuleY │
└──────────┘ └──────────────┘

APRÈS:
┌────────────────────────┐
│ main.cpp (propre) │
├────────────────────────┤
│ • robot.initialize() │
│ • robot.update() │
│ • robot.getState() │
│ • ~10 lignes │
│ • État centralisé │
│ • Facile à déboguer │
└────────────────────────┘
│
▼
┌──────────────┐
│ RobotSystem │
├──────────────┤
│ • RobotState │────┬─────────────┬─────────────┐
│ • MPU6050 │ │ │ │
│ • PID Ctrl │ ▼ ▼ ▼
│ • Motors │ Sensors Computation Output
└──────────────┘
│ │
▼ ▼
┌──────────────┐ ┌─────────────┐ ┌──────────────┐
│TimingControl │ │ Monitor │ │StateManager │
└──────────────┘ └─────────────┘ └──────────────┘

RÉSULTAT:
✓ Code lisible
✓ État visible
✓ Facile à tester
✓ Facile à étendre
✓ Facile à maintenir
\*/

// ═══════════════════════════════════════════════════════════════════════════
// GUIDE DE RÉFÉRENCE RAPIDE
// ═══════════════════════════════════════════════════════════════════════════

/\*
QUESTION: Comment faire ... ?

1. "Accéder à l'angle actuel"
   // AVANT:
   float angle = currentAngle;

   // APRÈS:
   float angle = robot.getState().imu.currentAngle;

2. "Changer les paramètres PID"
   // AVANT:
   KP = 0.5; KI = 0.1; KD = 0.05;

   // APRÈS:
   robot.setPidParameters(0.5, 0.1, 0.05, targetAngle);

3. "Logger les données"
   // AVANT:
   printDataOnTimer();

   // APRÈS:
   monitor.logData(robot.getState());

4. "Changer le format d'affichage"
   // AVANT:
   // (pas possible, hardcodé)

   // APRÈS:
   monitor.setOutputFormat(PerformanceMonitor::OutputFormat::DEBUG);

5. "Charger la config depuis SD"
   // AVANT:
   JSON2Client();

   // APRÈS:
   StateManager config;
   config.loadAllConfiguration(robot.getMutableState());

6. "Sauvegarder la config vers SD"
   // AVANT:
   JSON2Server();

   // APRÈS:
   StateManager config;
   config.saveAllConfiguration(robot.getState());

7. "Vérifier la fréquence de la boucle"
   // AVANT:
   // (impossible, pas de mesure)

   // APRÈS:
   float freq = timing.getLoopFrequencyHz();

8. "Déboguer un problème"
   // AVANT:
   // (ajouter des Serial.println() partout)
   // APRÈS:
   monitor.setVerbosity(3); // Verbose mode
   monitor.setOutputFormat(OutputFormat::DEBUG);
   Serial.println(robot.getDiagnostics()); // JSON diagnostics
   \*/

// ═══════════════════════════════════════════════════════════════════════════
// POINTS D'ATTENTION LORS DE LA MIGRATION
// ═══════════════════════════════════════════════════════════════════════════

/\*
⚠️ ATTENTION 1: Les variables extern n'existent plus
Remplacer tous les accès par: robot.getState().xxx

⚠️ ATTENTION 2: Les fonctions globales disparaissent
calculPID(), controlMotors(), applyFiltrage() → Dans RobotSystem

⚠️ ATTENTION 3: Les timers manuels disparaissent
timing.shouldExecutePID(), timing.shouldDisplayData()

⚠️ ATTENTION 4: StateManager doit être initialisé
Avant d'appeler loadConfiguration() ou saveConfiguration()

⚠️ ATTENTION 5: PerformanceMonitor doit être appelé chaque loop
Dans le if (timing.shouldDisplayData()) pour être efficace

⚠️ ATTENTION 6: Les changements de paramètres nécessitent une API
Utiliser les setters (setPidParameters, setJoystickInput, etc.)

⚠️ ATTENTION 7: Le delta time est maintenant calculé par TimingController
Plus besoin de le calculer manuellement: timing.getDeltaTimeSeconds()

⚠️ ATTENTION 8: L'état est déjà mis à jour par robot.update()
Pas besoin d'appeler updateIMU(), calculatePID(), etc. séparément
\*/

// ═══════════════════════════════════════════════════════════════════════════
// CHECKPOINTS DE COMPILATION
// ═══════════════════════════════════════════════════════════════════════════

/\*
Étape 1: Après ajout des headers
☐ Compilation réussie
☐ Pas d'erreurs de redéfinition
☐ Pas d'erreurs de circular includes

Étape 2: Après ajout de RobotState
☐ initializeRobotState() compile
☐ validateRobotState() compile
☐ Création d'une instance: RobotState state;

Étape 3: Après ajout de RobotSystem
☐ Création d'une instance: RobotSystem robot;
☐ robot.initialize() compile
☐ robot.update() compile

Étape 4: Après ajout de TimingController
☐ Création d'une instance: TimingController timing;
☐ timing.update() compile
☐ timing.shouldExecutePID() compile

Étape 5: Après ajout de PerformanceMonitor
☐ Création d'une instance: PerformanceMonitor monitor;
☐ monitor.logData() compile
☐ monitor.setOutputFormat() compile

Étape 6: Après ajout de StateManager
☐ Création d'une instance: StateManager config;
☐ config.initialize() compile
☐ config.loadAllConfiguration() compile

Étape 7: Upload et test sur ESP32
☐ Pas de crash au démarrage
☐ robot.initialize() se termine correctement
☐ loop() tourne sans erreur
☐ Les moteurs répondent
☐ Les données s'affichent correctement
\*/

// ═══════════════════════════════════════════════════════════════════════════
// NEXT STEPS
// ═══════════════════════════════════════════════════════════════════════════

/\*

1. ✓ Lire ce fichier (exemple complet)
2. → Compiler les nouveaux fichiers (sans les utiliser)
3. → Créer une branche git: git checkout -b refactor/step1-compilation
4. → Remplacer les timers (étape 1 de MIGRATION_GUIDE.md)
5. → Compiler et tester
6. → Committer: git commit -m "Step 1: Replace timers"
7. → Continuer les étapes...

BON COURAGE! 🚀
\*/
