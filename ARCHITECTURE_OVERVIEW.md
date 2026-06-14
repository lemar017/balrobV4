/\*\*

- ARCHITECTURE REFACTORISÉE - VUE D'ENSEMBLE
-
- Ce fichier montre visuellement comment les nouvelles classes
- s'articulent ensemble pour remplacer le chaos actuel.
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ARCHITECTURE ACTUELLE (AVANT)
// ═══════════════════════════════════════════════════════════════════════════

/\*
main.cpp
│
├─ #define [20 defines de debug]
├─ extern [50+ variables globales]
│
├─ setup()
│ ├─ Serial.begin(115200);
│ ├─ initSDCard();
│ ├─ initWiFi();
│ ├─ initMPU6050();
│ ├─ calibrateMPU6050();
│ ├─ initMotors();
│ ├─ initFilters();
│ ├─ initPID();
│ └─ initServerWeb();
│
└─ loop() [100+ lignes!]
├─ Vérification timers manuels
├─ Lire capteurs
├─ Calculer filtrage
├─ Calculer PID
├─ Contrôler moteurs
├─ Afficher données
├─ Gérer LED
└─ Traiter requêtes web

PROBLÈMES:
✗ Pas de structure clair
✗ Variables globales partout
✗ Responsabilités mélangées
✗ Difficile de tester
✗ Difficile d'étendre
✗ État du système invisible
\*/

// ═══════════════════════════════════════════════════════════════════════════
// NOUVELLE ARCHITECTURE (APRÈS)
// ═══════════════════════════════════════════════════════════════════════════

/\*
main.cpp (SIMPLE)
│
├─ RobotSystem robot;
├─ TimingController timing;
├─ PerformanceMonitor monitor;
│
├─ setup()
│ └─ robot.initialize();
│
└─ loop()
├─ timing.update();
├─ robot.update();
├─ if (timing.shouldDisplayData()) monitor.logData(robot.getState());
└─ if (timing.shouldToggleLED()) digitalWrite(LED, ...);

         │
         ├─────────────────────┬──────────────────────┬─────────────────────┐
         ▼                     ▼                      ▼                     ▼

    RobotSystem         TimingController      PerformanceMonitor    StateManager
    (Orchestrateur)     (Timing)              (Télémétrie)          (Persistance)
    │                   │                     │                     │
    ├─ RobotState       ├─ shouldExecutePID()├─ logData()          ├─ loadPID()
    ├─ MPU6050          ├─ shouldDisplay()   ├─ logIMUData()       ├─ savePID()
    ├─ BalanceCtrl      ├─ shouldToggleLED() ├─ logPIDData()       ├─ loadWiFi()
    ├─ MotorDriver      ├─ getDeltaTime()    ├─ setOutputFormat()  └─ saveWiFi()
    ├─ ImuFilter        └─ getFrequency()    └─ getStatistics()
    │
    ├─ initialize()
    ├─ update()
    ├─ setPidParameters()
    ├─ setJoystickInput()
    ├─ setFilterType()
    ├─ getState()
    └─ getDiagnostics()

         │
         └─ RobotState (DONNÉES CENTRALES)
            ├─ imu.raw / imu.filtered
            ├─ pid.params / pid.state
            ├─ motors.current
            ├─ joystick.current
            ├─ filter.type
            ├─ timing.deltaTime
            └─ system.status

AVANTAGES:
✓ Architecture claire et hiérarchique
✓ Une seule source de vérité (RobotState)
✓ Responsabilités bien séparées
✓ Chaque classe teste en isolation
✓ Facile d'étendre (ajouter une classe, pas modifier le reste)
✓ État du système visible et cohérent
\*/

// ═══════════════════════════════════════════════════════════════════════════
// FLUX DE DONNÉES
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT (chaotique):

Capteurs (MPU6050)
│
└─ mpu.getMotion6()
│
├─ accX, accY, accZ ──┐
├─ gyroX, gyroY, gyroZ├─ Variables GLOBALES EXTERN
│ │
├─ applyFiltrage()
│
├─ accX_filtered ──┐
├─ gyroY_filtered ├─ Plus de variables GLOBALES
│ │
└─ calculPID()
│
├─ targetAngle ──┐
├─ P, I, D ├─ Encore plus de variables!
│
└─ motorPID ──┐
├─ Utilisé pour controlMotors()
joystick ────┘
│
└─ Motors

APRÈS (propre):

Capteurs (MPU6050)
│
└─ RobotSystem.updateIMU\_()
│
└─ state.imu.raw = {accX, accY, accZ, gyroX, gyroY, gyroZ}
│
└─ ImuFilter.update(state.imu.raw)
│
└─ state.imu.filtered = {...}
│
└─ BalanceController.compute(state.imu.filtered, ...)
│
└─ state.pid.motorOutput = {...}
│
└─ MotorDriver.setSpeed()

┌───────────────────────────────────────────────────────┐
│ Tout passe par RobotState (UNE SEULE STRUCTURE) │
│ Facile à debugger: inspecter l'état à tout moment │
└───────────────────────────────────────────────────────┘

\*/

// ═══════════════════════════════════════════════════════════════════════════
// RESPONSABILITÉS DE CHAQUE CLASSE
// ═══════════════════════════════════════════════════════════════════════════

/\*
RobotState
├─ QUI: Structure de données
├─ QUOI: Contient l'état complet du robot
├─ RESPONSABILITÉS:
│ ├─ IMU (capteurs bruts et filtrés)
│ ├─ PID (paramètres et état)
│ ├─ Moteurs (commandes courantes)
│ ├─ Joystick (entrée utilisateur)
│ └─ Timing (delta time)
└─ TESTS: Valider qu'un RobotState est cohérent

RobotSystem
├─ QUI: Orchestrateur
├─ QUOI: Dirige tous les composants
├─ RESPONSABILITÉS:
│ ├─ Initialiser tous les sous-systèmes
│ ├─ Mettre à jour l'état du robot
│ ├─ Exposer des interfaces pour les modules externes
│ └─ Maintenir la cohérence de l'état
└─ TESTS:
├─ initialize() crée tous les composants
├─ update() ne plante jamais
├─ setPidParameters() met à jour correctement
└─ getDiagnostics() retourne des infos sensibles

TimingController
├─ QUI: Gestionnaire de temps
├─ QUOI: Détermine quand exécuter chaque tâche
├─ RESPONSABILITÉS:
│ ├─ Calculer delta time
│ ├─ Vérifier si chaque tâche doit s'exécuter
│ └─ Calculer les statistiques de fréquence
└─ TESTS:
├─ shouldExecutePID() respects l'intervalle
├─ getDeltaTime() est cohérent
└─ getLoopFrequency() est correct

PerformanceMonitor
├─ QUI: Système de télémétrie
├─ QUOI: Affiche et enregistre les données
├─ RESPONSABILITÉS:
│ ├─ Formatter les données selon le format choisi
│ ├─ Envoyer vers Serial
│ └─ Calculer les statistiques de logging
└─ TESTS:
├─ Chaque format affiche les bonnes données
├─ setOutputFormat() change le format
└─ getLogCount() est exact

StateManager
├─ QUI: Gestionnaire de configuration
├─ QUOI: Charge et sauvegarde la configuration
├─ RESPONSABILITÉS:
│ ├─ Lire/écrire les fichiers JSON sur SD
│ ├─ Valider les paramètres
│ └─ Exposer des interfaces web pour les mise-à-jour
└─ TESTS:
├─ loadPID() charge le bon fichier
├─ savePID() écrit le bon format
├─ validatePIDParameters() détecte les valeurs invalides
└─ Récupération après erreur SD

\*/

// ═══════════════════════════════════════════════════════════════════════════
// EXEMPLE: AJOUTER UNE NOUVELLE FEATURE
// ═══════════════════════════════════════════════════════════════════════════

/\*
SCÉNARIO: Ajouter un accéléromètre externe (capteur additionnel)

AVANT (chaos):

1. Ajouter des variables globales:
   extern int16_t acc2X, acc2Y, acc2Z;
   extern float acc2X_filtered, ...;

2. Modifier initMPU6050():
   mpu2.initialize(); // Nouvel I2C

3. Modifier loop():
   mpu2.getMotion3(acc2X, acc2Y, acc2Z);
   applyFiltrage2(); // Nouvelle fonction

4. Modifier les printData\*():
   Serial.print(acc2X);
   ...

5. Modifier les handlers web
   Résultat: Modifications dans 10+ fichiers!

APRÈS (propre):

1. Ajouter au RobotState:
   struct {
   ImuRaw raw;
   ImuFiltered filtered;
   } externalIMU; // Nouveau!

2. Ajouter à RobotSystem:
   bool initializeExternalIMU*();
   void updateExternalIMU*();

3. Dans RobotSystem::initialize():
   if (!initializeExternalIMU\_()) { ... }

4. Dans RobotSystem::update():
   updateExternalIMU\_();

5. Modifications totales: 1 fichier (RobotState.h, RobotSystem.cpp)!

Résultat: Extensible sans modifier le code existant!

\*/

// ═══════════════════════════════════════════════════════════════════════════
// COMPARAISON: TAILLE DU CODE
// ═══════════════════════════════════════════════════════════════════════════

/\*
Métrique: Nombre de lignes + Complexité

AVANT:
├─ main.cpp: ~500 lignes
├─ ModuleVAR-GLOB.h/cpp: ~200 lignes
├─ Module-WIFI.h/cpp: ~150 lignes
├─ Module-SDCARD.h/cpp: ~150 lignes
├─ Various print functions: ~100 lignes
└─ TOTAL: ~1100 lignes non-modulaire

APRÈS:
├─ main.cpp: ~150 lignes (75% réduction!)
├─ RobotSystem.h/cpp: ~300 lignes
├─ TimingController.h/cpp: ~150 lignes
├─ StateManager.h/cpp: ~250 lignes
├─ PerformanceMonitor.h/cpp: ~200 lignes
├─ RobotState.h/cpp: ~150 lignes
└─ TOTAL: ~1200 lignes MAIS modulaire!

MAIS:

- main.cpp: 75% plus court
- Chaque classe peut être testée seule
- Plus facile à comprendre
- Pas de variables globales cachées
- État du système visible

\*/

// ═══════════════════════════════════════════════════════════════════════════
// DEPENDENCIES ENTRE CLASSES
// ═══════════════════════════════════════════════════════════════════════════

/\*
RobotState
│
└─ (aucune dépendance) ✓ Zéro couplage!

RobotSystem
├─ dépend de: RobotState
├─ dépend de: BalanceController (existant)
├─ dépend de: MotorDriver (existant)
├─ dépend de: ImuFilter (existant)
├─ dépend de: MPU6050 (existant)
└─ utilisé par: main.cpp

TimingController
├─ dépend de: (aucune) ✓ Complètement indépendant!
└─ utilisé par: main.cpp

PerformanceMonitor
├─ dépend de: RobotState
└─ utilisé par: main.cpp

StateManager
├─ dépend de: RobotState
├─ dépend de: SD.h, ArduinoJson.h
└─ utilisé par: RobotSystem ou main.cpp

RÉSEAU DE DÉPENDANCES:
main.cpp
│
├─→ RobotSystem ──→ RobotState
│ │
├─→ TimingController (indépendant)
│
├─→ PerformanceMonitor ──→ RobotState
│
└─→ StateManager ──→ RobotState

AVANTAGE: Dépendances linéaires, pas de cycles!
\*/

// ═══════════════════════════════════════════════════════════════════════════
// CHECKLIST DÉTAILLÉE
// ═══════════════════════════════════════════════════════════════════════════

/\*
FICHIERS CRÉÉS:
✓ include/RobotState.h
✓ include/RobotSystem.h
✓ include/TimingController.h
✓ include/StateManager.h
✓ include/PerformanceMonitor.h

✓ src/RobotState.cpp
✓ src/RobotSystem.cpp
✓ src/TimingController.cpp
✓ src/StateManager.cpp
✓ src/PerformanceMonitor.cpp

✓ src/main_REFACTORED.cpp (exemple)
✓ MIGRATION_GUIDE.md (guide détaillé)
✓ ARCHITECTURE_OVERVIEW.md (ce fichier)

PROCHAINES ÉTAPES:

1. Lire ce fichier pour comprendre la vision
2. Lire MIGRATION_GUIDE.md pour la stratégie
3. Lire main_REFACTORED.cpp pour un exemple
4. Commencer la migration par étapes
5. Compiler et tester souvent

SUPPORT:

- Si une classe n'est pas claire, relire les commentaires
- Si une étape de migration est difficile, la sauter et revenir plus tard
- Si quelque chose semble mal, valider que c'est juste la structure
- Les tests doivent être écrits APRÈS la migration complète
  \*/
