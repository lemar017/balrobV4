// Script JavaScript pour l'interface PID
// Gestion de l'affichage et communication avec l'ESP32

// Variables globales pour stocker les paramètres
let pidParams = {};

// 1- Fonction pour charger les données depuis l'ESP32 (qui lit pid.json)
async function loadPidData() {
  try {
    console.log("Chargement des données PID...");
    const response = await fetch("/getpid");
    const data = await response.json();
    pidParams = data;
    console.log("pidParams dans loadpidata =", pidParams);

    // 2- Remplir les objets de la page HTML avec les valeurs du fichier JSON
    updateHTMLElements();

    // 3- Positionner les curseurs des sliders
    updateSliders();

    showStatus("Données chargées avec succès", "success");
    console.log("Initialisation terminée");
  } catch (error) {
    showStatus("Erreur lors du chargement des données", "error");
    console.error("Erreur:", error);
  }
}

// 2- Fonction pour mettre à jour les éléments HTML avec les valeurs du JSON
function updateHTMLElements() {
  // Mettre à jour les champs input number (min/max)
  document.getElementById("TAmin").value = pidParams.TAmin;
  document.getElementById("TAmax").value = pidParams.TAmax;
  document.getElementById("KPmin").value = pidParams.KPmin;
  document.getElementById("KPmax").value = pidParams.KPmax;
  document.getElementById("KImin").value = pidParams.KImin;
  document.getElementById("KImax").value = pidParams.KImax;
  document.getElementById("KDmin").value = pidParams.KDmin;
  document.getElementById("KDmax").value = pidParams.KDmax;

  // Mettre à jour les zones d'affichage des valeurs (val)
  document.getElementById("TAval").value = pidParams.TAval.toFixed(1);
  document.getElementById("KPval").value = pidParams.KPval.toFixed(1);
  document.getElementById("KIval").value = pidParams.KIval.toFixed(1);
  document.getElementById("KDval").value = pidParams.KDval.toFixed(1);

  // Affichage dans la console pour vérification
  console.log("Valeurs affichées dans les champs:");
  console.log(
    "TAmin:",
    pidParams.TAmin,
    "TAmax:",
    pidParams.TAmax,
    "TAval:",
    pidParams.TAval
  );
  console.log(
    "KPmin:",
    pidParams.KPmin,
    "KPmax:",
    pidParams.KPmax,
    "KPval:",
    pidParams.KPval
  );
  console.log(
    "KImin:",
    pidParams.KImin,
    "KImax:",
    pidParams.KImax,
    "KIval:",
    pidParams.KIval
  );
  console.log(
    "KDmin:",
    pidParams.KDmin,
    "KDmax:",
    pidParams.KDmax,
    "KDval:",
    pidParams.KDval
  );
}

// 3- Fonction pour positionner les sliders
function updateSliders() {
  // Configurer le slider TA
  const taSlider = document.getElementById("TArange");
  taSlider.min = pidParams.TAmin;
  taSlider.max = pidParams.TAmax;
  taSlider.value = pidParams.TAval;
  document.getElementById("TAval").value = pidParams.TAval.toFixed(1);

  // Configurer le slider KP
  const kpSlider = document.getElementById("KPrange");
  kpSlider.min = pidParams.KPmin;
  kpSlider.max = pidParams.KPmax;
  kpSlider.value = pidParams.KPval;
  document.getElementById("KPval").value = pidParams.KPval.toFixed(1);

  // Configurer le slider KI
  const kiSlider = document.getElementById("KIrange");
  kiSlider.min = pidParams.KImin;
  kiSlider.max = pidParams.KImax;
  kiSlider.value = pidParams.KIval;
  document.getElementById("KIval").value = pidParams.KIval.toFixed(1);

  // Configurer le slider KD
  const kdSlider = document.getElementById("KDrange");
  kdSlider.min = pidParams.KDmin;
  kdSlider.max = pidParams.KDmax;
  kdSlider.value = pidParams.KDval;
  document.getElementById("KDval").value = pidParams.KDval.toFixed(1);

  console.log("Sliders configurés avec les valeurs:", {
    TA: pidParams.TAval,
    KP: pidParams.KPval,
    KI: pidParams.KIval,
    KD: pidParams.KDval,
  });
}

// 4- Fonction pour envoyer les valeurs modifiées en TR vers l'ESP32
async function sendSliderValue(parameter, value) {
  try {
    const response = await fetch("/updatePID", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        parameter: parameter,
        value: parseFloat(value),
      }),
    });

    if (response.ok) {
      // Mettre à jour la valeur localement
      pidParams[parameter] = parseFloat(value);

      // Mettre à jour l'affichage de la valeur dans la zone correspondante
      const valueElement = document.getElementById(parameter);
      if (valueElement) {
        valueElement.textContent = parseFloat(value).toFixed(1);
        console.log(
          "Valeur mise à jour:",
          parameter,
          "=",
          parseFloat(value).toFixed(1)
        );
      } else {
        console.error("Élément non trouvé:", parameter);
      }
    } else {
      console.error("Erreur lors de l'envoi vers l'ESP32");
    }
  } catch (error) {
    console.error("Erreur lors de l'envoi:", error);
  }
}

// 5- Fonction pour adapter les limites des sliders

function updateSliderLimits(prefix) {
  const minElement = document.getElementById(prefix + "min");
  const maxElement = document.getElementById(prefix + "max");
  const rangeElement = document.getElementById(prefix + "range");

  if (minElement && maxElement && rangeElement) {
    const minValue = parseFloat(minElement.value);
    const maxValue = parseFloat(maxElement.value);

    if (!isNaN(minValue) && !isNaN(maxValue) && minValue < maxValue) {
      rangeElement.min = minValue;
      rangeElement.max = maxValue;

      // Ajuster la valeur actuelle si elle est hors limites
      const currentValue = parseFloat(rangeElement.value);
      if (currentValue < minValue) {
        rangeElement.value = minValue;
        sendSliderValue(prefix + "val", minValue);
      } else if (currentValue > maxValue) {
        rangeElement.value = maxValue;
        sendSliderValue(prefix + "val", maxValue);
      }

      // Mettre à jour les paramètres locaux
      pidParams[prefix + "min"] = minValue;
      pidParams[prefix + "max"] = maxValue;
    }
  }
}

// 6- Fonction pour sauvegarder sur la carte SD
async function saveToSD() {
  try {
    // Mettre à jour pidParams avec les valeurs actuelles des champs
    pidParams.TAval = parseFloat(document.getElementById("TAval").value);
    pidParams.KPval = parseFloat(document.getElementById("KPval").value);
    pidParams.KIval = parseFloat(document.getElementById("KIval").value);
    pidParams.KDval = parseFloat(document.getElementById("KDval").value);
    
    pidParams.TAmin = parseFloat(document.getElementById("TAmin").value);
    pidParams.TAmax = parseFloat(document.getElementById("TAmax").value);
    pidParams.KPmin = parseFloat(document.getElementById("KPmin").value);
    pidParams.KPmax = parseFloat(document.getElementById("KPmax").value);
    pidParams.KImin = parseFloat(document.getElementById("KImin").value);
    pidParams.KImax = parseFloat(document.getElementById("KImax").value);
    pidParams.KDmin = parseFloat(document.getElementById("KDmin").value);
    pidParams.KDmax = parseFloat(document.getElementById("KDmax").value);

    console.log("Données à sauvegarder:", pidParams);

    const response = await fetch("/pidsave", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(pidParams),
    });

    if (response.ok) {
      showStatus("Données sauvegardées sur la carte SD", "success");
    } else {
      showStatus("Erreur lors de la sauvegarde", "error");
    }
  } catch (error) {
    showStatus("Erreur lors de la sauvegarde", "error");
    console.error("Erreur:", error);
  }
}

// Fonction utilitaire pour afficher les messages de statut
function showStatus(message, type) {
  const statusElement = document.getElementById("status");
  statusElement.textContent = message;
  statusElement.className = `status ${type}`;
  statusElement.style.display = "block";

  // Masquer le message après 3 secondes
  setTimeout(() => {
    statusElement.style.display = "none";
  }, 3000);
}

// Initialisation des événements
document.addEventListener("DOMContentLoaded", function () {
  // Charger les données au démarrage
  loadPidData();

  // 4- Événements pour les sliders
  ["TA", "KP", "KI", "KD"].forEach((prefix) => {
    const slider = document.getElementById(prefix + "range");
    const valueDisplay = document.getElementById(prefix + "val");

    if (slider && valueDisplay) {
      // Événement pour mise à jour en temps réel pendant le déplacement
      slider.addEventListener("input", function () {
        // Mettre à jour l'affichage immédiatement
        valueDisplay.value = parseFloat(this.value).toFixed(1);
        // Envoyer la valeur à l'ESP32
        sendSliderValue(prefix + "val", this.value);
      });

      // Événement pour la fin du déplacement (optionnel, pour confirmation)
      slider.addEventListener("change", function () {
        console.log(`${prefix} slider changé à: ${this.value}`);
      });
    }
  });

  // 5- Événements pour les champs min/max
  ["TA", "KP", "KI", "KD"].forEach((prefix) => {
    const minElement = document.getElementById(prefix + "min");
    const maxElement = document.getElementById(prefix + "max");

    if (minElement) {
      minElement.addEventListener("change", function () {
        updateSliderLimits(prefix);
      });
    }

    if (maxElement) {
      maxElement.addEventListener("change", function () {
        updateSliderLimits(prefix);
      });
    }
  });

  // 6- Événement pour le bouton de sauvegarde
const saveButton = document.getElementById("save2SD");
console.log('Bouton trouvé:', saveButton);
console.log('Type de saveButton:', typeof saveButton);
console.log('saveButton est null?', saveButton === null);
console.log('saveButton est undefined?', saveButton === undefined);
console.log('Evaluation if (saveButton):', !!saveButton);

console.log('On passe dans la programmation evenement bouton save');

if (saveButton) {
    console.log('ON ENTRE DANS LE IF - ATTACHEMENT EVENEMENT');
    saveButton.addEventListener("click", saveToSD);
} else {
    console.log('ON N ENTRE PAS DANS LE IF');
}
 
});


// -----------------Appel getData() au chargement de la page
window.onload = function () {
  console.log(
    "------------------------On passe dans window.onload---------------------"
  );
 loadPidData();
};



// Fonction pour rafraîchir périodiquement les données (optionnel)
//setInterval(loadPidData, 30000); // Rafraîchir toutes les 30 secondes
