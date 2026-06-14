
// Obtenir l'hôte (nom de domaine) du serveur
var serverHost = window.location.hostname;

// Obtenir le protocole utilisé (http:// ou https://)
var serverProtocol = window.location.protocol;

// Obtenir le port du serveur
var serverPort = window.location.port;

// Construire l'URL complète du serveur
var serverURL = serverProtocol + '//' + serverHost;
if (serverPort) {
    serverURL += ':' + serverPort
};
console.log("URL du serveur:", serverURL);

