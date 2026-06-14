/\*\*

- GUIDE DE MIGRATION - REFACTORISATION PROGRESSIVE
-
- Ce guide vous montre comment migrer progressivement de l'ancienne
- architecture à la nouvelle, sans casser la fonctionnalité existante.
-
- PHASING:
- ========
- Phase 1: Préparation - Créer les nouvelles classes (FAIT ✓)
- Phase 2: Cohabitation - Anciens ET nouveaux côte à côte
- Phase 3: Migration progressive - Remplacer morceau par morceau
- Phase 4: Nettoyage - Supprimer l'ancien code
- Phase 5: Optimisation - Refiner et améliorer
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 1: INTÉGRATION LÉGÈRE (1-2 jours)
// ═══════════════════════════════════════════════════════════════════════════

/\*
OBJECTIF: Mettre les nouvelles classes en place sans modifier main.cpp

TÂCHES:

1. ✓ Créer tous les fichiers .h en include/
2. ✓ Créer tous les fichiers .cpp en src/
3. Ajouter au platformio.ini:
   [env:nodemcu-32s]
   build_flags =
   -DDEBUG=0
   -DUSE_NEW_SYSTEM=1
   lib_deps =
   ... (déjà existants)

4. Inclure les nouvelles headers en haut de main.cpp:
   #include "RobotState.h"
   #include "RobotSystem.h"
   #include "TimingController.h"
   #include "StateManager.h"
   #include "PerformanceMonitor.h"

5. Compiler et vérifier qu'il n'y a pas d'erreurs
   \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 2: REMPLACER LES TIMERS (1 jour)
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT:
unsigned long dernierPID = 0;
unsigned long dernierAffichage = 0;
unsigned long dernierLED = 0;
const unsigned long INTERVALLE_PID = 10000;
const unsigned long INTERVALLE_AFFICHAGE = 20000;
const unsigned long INTERVALLE_LED = 500000;

APRÈS:
TimingController timing(10000, 20000, 500000);

DANS setup():
timing.update() doit être appelé CHAQUE LOOP

DANS loop():
// À la place de:
if (now - dernierPID >= INTERVALLE_PID) { ... }

// Utiliser:
if (timing.shouldExecutePID()) { ... }

FICHIERS À MODIFIER: src/main.cpp seulement
RISQUE: FAIBLE (les timers sont isolés)
\*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 3: CRÉER RobotState ET PEUPLER PROGRESSIVEMENT (2-3 jours)
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT:
extern int16_t accX, accY, accZ, gyroX, gyroY, gyroZ;
extern float accX_filtered, ...;
extern double targetAngle;
extern double KP, KI, KD;
// 50+ variables extern

APRÈS:
RobotState state; // Une seule variable contenant TOUT

MIGRATION:

1. Créer RobotState vide
2. Remplacer PROGRESSIVEMENT chaque variable:

   // ancien:
   accX = 100;

   // nouveau:
   state.imu.raw.accX = 100;

3. Utiliser des #define aliases pour les anciennes variables:
   #define accX state.imu.raw.accX
   #define KP state.pid.params.kp

   ⚠️ Attention: Les aliases ne permettent pas de passer par référence!
   Donc certains endroits devront être modifiés.

FICHIERS À MODIFIER:

- include/RobotState.h (structure)
- src/main.cpp (initialisation et utilisation)
- src/ModuleVAR-GLOB.cpp (mettre à jour les externs vers le state)

STRATÉGIE DE TEST:

- Compiler et tester à chaque groupe de 5-10 variables
- Garder l'ancien code commenté en cas de rollback
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 4: INTÉGRER RobotSystem PROGRESSIVEMENT (3-4 jours)
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT:
void setup() {
Serial.begin(115200);
initSDCard();
initWiFi();
initMPU6050();
// ... 30 lignes
}

APRÈS:
RobotSystem robot;

void setup() {
robot.initialize(); // Contient tout
}

MIGRATION:

1. Créer une instance globale:
   RobotSystem robot;

2. Appeler robot.initialize() en setup()

3. REMPLACER les appels dans loop():
   // ancien:
   updateIMU();
   calculatePID();
   controlMotors();

   // nouveau:
   robot.update(); // Contient tout

4. Migrer les handlers web:
   - Avant: receivePIDenTR(), handleWiFiSettings(), etc.
   - Après: Utiliser les setters de RobotSystem
     robot.setPidParameters(kp, ki, kd, ta);

FICHIERS À MODIFIER:

- src/main.cpp (setup/loop)
- src/RobotSystem.cpp (implémenter les méthodes privées)
- include/Module-WIFI.h, Module-SDCARD.h (adapter aux nouvelles interfaces)

STRATÉGIE DE TEST:

- Garder l'ancien code et utiliser des #ifdef OLD_SYSTEM
- Commencer par l'initialisation (setup)
- Puis la boucle principale (loop)
- Puis les handlers web
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 5: INTÉGRER StateManager (2-3 jours)
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT:
JSON2Client(); // Charge depuis SD
JSON2Server(); // Sauvegarde vers SD

APRÈS:
StateManager configManager;
configManager.loadAllConfiguration(state);
configManager.saveAllConfiguration(state);

MIGRATION:

1. Créer une instance dans RobotSystem:
   StateManager stateManager\_;

2. Remplacer les appels:
   // ancien:
   JSON2Client();

   // nouveau:
   stateManager*.loadPIDConfiguration(state*);

3. Adapter les handlers web:
   // ancien:
   server.on("/api/pid", handleUpdatePID);

   // nouveau:
   stateManager\_.setupWebHandlers(server);

FICHIERS À MODIFIER:

- src/RobotSystem.cpp (intégrer StateManager)
- src/StateManager.cpp (implémenter JSON parsing)
- include/Module-SDCARD.h (adapter si nécessaire)

STRATÉGIE DE TEST:

- Tester le chargement seul d'abord
- Puis la sauvegarde seul
- Puis les deux ensemble
- Vérifier que les fichiers SD ne sont pas corrompus
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 6: REMPLACER L'AFFICHAGE (1-2 jours)
// ═══════════════════════════════════════════════════════════════════════════

/\*
AVANT:
void printData() { Serial.println(...); }
void printDataOnTimer() { Serial.println(...); }
void printDataBinaire() { Serial.write(...); }
void printDataFiltree() { Serial.println(...); }
// ... 10+ fonctions similaires

APRÈS:
PerformanceMonitor monitor(OutputFormat::CSV);
if (timing.shouldDisplayData()) {
monitor.logData(state);
}

MIGRATION:

1. Créer une instance globale:
   PerformanceMonitor monitor;

2. Remplacer les appels:
   // ancien:
   if (timing.shouldDisplayData()) {
   printDataOnTimer();
   }

   // nouveau:
   if (timing.shouldDisplayData()) {
   monitor.logData(robot.getState());
   }

3. Changer le format depuis l'interface web:
   monitor.setOutputFormat(OutputFormat::DEBUG);

FICHIERS À MODIFIER:

- src/main.cpp (remplacer les printData\*() calls)
- Optionnel: Garder les anciennes fonctions pour la compatibilité

STRATÉGIE DE TEST:

- Vérifier que les données affichées sont identiques
- Tester les différents formats (RAW, CSV, DEBUG, etc.)
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// ÉTAPE 7: NETTOYAGE ET OPTIMISATION (1-2 jours)
// ═══════════════════════════════════════════════════════════════════════════

/\*
UNE FOIS QUE TOUT FONCTIONNE:

1. Supprimer le code ancien:
   - Supprimer ModuleVAR-GLOB.h (remplacé par RobotState)
   - Supprimer les printData\*() functions
   - Nettoyer les #ifdef OLD_SYSTEM
2. Vérifier les compilateurs flags:
   - Retirer les #define DEBUG\_\*
   - Optimiser les log levels

3. Tester en conditions réelles:
   - Vérifier les performances
   - Vérifier la mémoire
   - Vérifier la stabilité

4. Documenter les changements:
   - Créer un README avec la nouvelle architecture
   - Documenter les APIs web
     \*/

// ═══════════════════════════════════════════════════════════════════════════
// TIMELINE ESTIMÉE
// ═══════════════════════════════════════════════════════════════════════════

/\*
Jour 1: Étape 1 (intégration légère)
Jour 2: Étape 2 (timers)
Jour 3-4: Étape 3 (RobotState)
Jour 5-6: Étape 4 (RobotSystem)
Jour 7-8: Étape 5 (StateManager)
Jour 9: Étape 6 (affichage)
Jour 10: Étape 7 (nettoyage)

TOTAL: 10 jours (si 1-2h par jour = 2-3 semaines à temps partiel)

RISQUE: MODÉRÉ

- Tester à chaque étape
- Garder des backups
- Compiler souvent
  \*/

// ═══════════════════════════════════════════════════════════════════════════
// CHECKLIST DE MIGRATION
// ═══════════════════════════════════════════════════════════════════════════

/\*
Phase 1 - Préparation:
☐ Créer tous les fichiers .h et .cpp
☐ Compiler sans erreur
☐ Tester les compilations incrémentales

Phase 2 - Timers:
☐ Remplacer les timers globaux
☐ Vérifier que les intervalles sont respectés
☐ Tester sur hardware (ESP32)

Phase 3 - RobotState:
☐ Créer la structure RobotState
☐ Remplacer les variables extern progressivement
☐ Tester la cohérence des données
☐ Vérifier que le robot répond toujours

Phase 4 - RobotSystem:
☐ Intégrer RobotSystem
☐ Simplifier setup()
☐ Simplifier loop()
☐ Tester les handlers web

Phase 5 - StateManager:
☐ Intégrer StateManager
☐ Tester le chargement depuis SD
☐ Tester la sauvegarde vers SD
☐ Vérifier les fichiers JSON

Phase 6 - PerformanceMonitor:
☐ Remplacer les printData\*()
☐ Tester tous les formats d'affichage
☐ Vérifier que la sérialité n'est pas saturée

Phase 7 - Nettoyage:
☐ Supprimer le code ancien
☐ Vérifier qu'il n'y a pas de code mort
☐ Compiler avec les warnings max
☐ Dernier test complet

FINAL:
☐ Backup du code ancien (git tag)
☐ Documentation mise à jour
☐ README réécrit
\*/

// ═══════════════════════════════════════════════════════════════════════════
// CONSEILS D'OR
// ═══════════════════════════════════════════════════════════════════════════

/\*

1. NE PAS REFACTORISER 100% D'UN COUP
   → Risque d'avoir un code cassé pendant longtemps
   → Impossible de déboguer si tout change simultanément

2. COMPILER ET TESTER SOUVENT
   → Après chaque étape, compiler et vérifier
   → Garder le code en état de fonctionnement

3. GARDER DES BRANCHES GIT
   → git checkout -b refactor/robot-system
   → git commit après chaque étape
   → Facile de revenir en arrière

4. DOCUMENTER LES CHANGEMENTS
   → Ajouter des commentaires aux nouveaux codes
   → Garder trace des décisions

5. NE PAS AVOIR PEUR DE DEMANDER DE L'AIDE
   → Si quelque chose semble trop compliqué
   → Demander des clarifications

6. TESTER PROGRESSIVEMENT
   → Phase 1: Compilation
   → Phase 2: Upload et démarrage
   → Phase 3: Comportement identique
   → Phase 4: Performance
   → Phase 5: Nouvelles features

7. GARDER LE CODE SIMPLE
   → La refactorisation n'est pas une excuse pour compliquer
   → Plus simple = mieux
   \*/
