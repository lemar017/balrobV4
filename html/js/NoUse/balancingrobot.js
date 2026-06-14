//----------------------------envoi des données JSON vers le serveur------------
async function saveJSON2Server() {
  console.log(
    "------------------------On passe dans saveJSON2Server---------------------"
  );
  const TAmin = document.getElementById("TAmin").value;
  const TAmax = document.getElementById("TAmax").value;
  const TAval = document.getElementById("TAval").value;

  const KPmin = document.getElementById("KPmin").value;
  const KPmax = document.getElementById("KPmax").value;
  const KPval = document.getElementById("KPval").value;

  const KImin = document.getElementById("KImin").value;
  const KImax = document.getElementById("KImax").value;
  const KIval = document.getElementById("KIval").value;

  const KDmin = document.getElementById("KDmin").value;
  const KDmax = document.getElementById("KDmax").value;
  const KDval = document.getElementById("KDval").value;

  const response = await fetch("/pid", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      TAmin,
      TAmax,
      TAval,
      KPmin,
      KPmax,
      KPval,
      KImin,
      KImax,
      KIval,
      KDmin,
      KDmax,
      KDval,
    }),
  });

  if (response.ok) {
    alert("Data updated successfully");
  } else {
    alert("Failed to update data");
  }
 
}

//On affiche dans les zones texte les valeurs du fichier JSON de la carte SD---
function updateRangeValues(minInputId, maxInputId, rangeInputId) {
  var minInput = document.getElementById(minInputId);
  var maxInput = document.getElementById(maxInputId);
  var rangeInput = document.getElementById(rangeInputId);

  console.log("---------------------On passe dans updateRangeValues------------");

  rangeInput.setAttribute("min", minInput.value);
  rangeInput.setAttribute("max", maxInput.value);
  console.log("min =", minInput);
  console.log("max =", maxInput);
}
//------------Affiche la valeur du curseur dans le champ texte val------------------------------------------------------
function updatePosition(rangeInputId, rangeValueId) {
  //rangeInputId : nom de l'ascenseur concerné
  //rangeValueId : nom de la zone texte recevant la valeur du curseur
  var rangeInput = document.getElementById(rangeInputId);
  var rangeValue = document.getElementById(rangeValueId);

  console.log("-------------On passe dans updatePosition------------" );
  console.log("rangeInput =", rangeInput);
  console.log("rangeValue =", rangeValue);
  rangeValue.value = rangeInput.value;
  console.log("rangeValue =", rangeValue.value);
  console.log("sendPosPotar rangeInputId = ",rangeInputId);
  console.log("sendPosPotar rangeValue = ",rangeValue.value);
  //Envoi valeur vers serveur
  sendValue(rangeInputId,rangeValue.value);


  

}

//------------ Requête AJAX pour obtenir les données JSON de la carte SD--------
// et les afficher dans les champs texte
function loadJSONFromServer() {
  console.log("------------On passe dans loadJSONFromServer-------------");
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/getData", true);
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      //var data = JSON.stringify(xhr.responseText);
      //var data = JSON.parse(data);
      var data = JSON.parse(xhr.responseText);
      document.getElementById("TAmin").value = data.TAmin;
      document.getElementById("TAmax").value = data.TAmax;
      document.getElementById("TAval").value = data.TAval;
      document.getElementById("KPmin").value = data.KPmin;
      document.getElementById("KPmax").value = data.KPmax;
      document.getElementById("KPval").value = data.KPval;
      document.getElementById("KImin").value = data.KImin;
      document.getElementById("KImax").value = data.KImax;
      document.getElementById("KIval").value = data.KIval;
      document.getElementById("KDmin").value = data.KDmin;
      document.getElementById("KDmax").value = data.KDmax;
      document.getElementById("KDval").value = data.KDval; 

      //updateFields(data); 
    }
  };
  xhr.send();
}
//--------------------------Envoi position curseur au serveur----------------

function sendValue(id, value) {
  fetch(`/updatePID?range=${id}&value=${value}`)
      .then(response => response.text())
      .then(data => console.log(data));
      console.log("on passe dans sendvalue");
}

function updateRange(id) {
  let value = document.getElementById(id).value;
  sendValue(id, value);
  console.log("on passe dans updateRange");
}

// -----------------Appel getData() au chargement de la page
window.onload = function () {
  console.log(
    "------------------------On passe dans window.onload---------------------"
  );
  loadJSONFromServer();
};

