// Item 3: Fichier JavaScript indépendant

document.addEventListener("DOMContentLoaded", function () {
  // Item 7: Charger les données du filtre au chargement de la page
  loadFilterData();

  // Gestionnaire pour le formulaire
  document
    .getElementById("filterForm")
    .addEventListener("submit", function (e) {
      e.preventDefault();
      saveFilter();
    });

  // Gestionnaire pour le bouton Annuler
  document
    .getElementById("cancelButton")
    .addEventListener("click", function () {
      // Item 9: Retour à la page index.html
      window.location.href = "/";
    });
});

// Item 7: Charger les données du filtre depuis le JSON
function loadFilterData() {
  fetch("/json/filter.json")
    .then((response) => response.json())
    .then((data) => {
      document.getElementById("filter").value = data.filter || "MOYENNE";
      document.getElementById("interval").value = data.interval || 1000;
    })
    .catch((error) => {
      console.error("Erreur lors du chargement des données:", error);
    });
}

// Item 8: Sauvegarder les données du filtre
function saveFilter() {
  const filter = document.getElementById("filter").value;
  const interval = document.getElementById("interval").value;

  // Afficher l'indicateur de chargement
  document.getElementById("loading").style.display = "block";
  document.getElementById("statusMessage").textContent = "";

  // Envoyer les données au serveur
  fetch("/saveFilter", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: `filter=${encodeURIComponent(filter)}&interval=${encodeURIComponent(
      interval
    )}`,
  })
    .then((response) => response.json())
    .then((data) => {
      if (data.status === "success") {
        // Afficher le message de confirmation spécifique
        document.getElementById("statusMessage").textContent =
          "JSON mis à jour sur la carte SD";
        document.getElementById("statusMessage").style.color = "green";

        // Redirection après un court délai
        setTimeout(() => {
          // Item 9: Retour à la page index.html après sauvegarde
          window.location.href = "/";
        }, 1500);
      } else {
        throw new Error("Erreur lors de la sauvegarde");
      }
    })
    .catch((error) => {
      console.error("Erreur:", error);
      document.getElementById("statusMessage").textContent =
        "Erreur lors de la sauvegarde. Veuillez réessayer.";
      document.getElementById("statusMessage").style.color = "red";
    })
    .finally(() => {
      // Cacher l'indicateur de chargement
      document.getElementById("loading").style.display = "none";
    });
}
