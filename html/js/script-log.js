// WebSocket Logger: Connexion au WebSocket /log pour afficher les logs en temps réel
      (function () {
        const host = location.hostname; // récupère l'hôte courant (ESP32)
        const wsUrl = "ws://" + host + "/log";
        let ws;
        let reconnectTimeout = 2000;
        let maxReconnectTimeout = 30000; // max 30 secondes entre tentatives
        let reconnectAttempts = 0;
        let maxLogLines = 500; // limite du nombre de lignes affichées

        const logDiv = document.getElementById("log");
        let logLineCount = 0;

        function appendLogLine(line) {
          if (!logDiv) return;

          // Supprimer le contenu initial si présent
          if (logLineCount === 0) {
            logDiv.innerHTML = "";
          }

          // créer un nouvel élément de ligne de log
          const p = document.createElement("p");
          p.textContent = new Date().toLocaleTimeString() + " -> " + line;
          logDiv.appendChild(p);
          logLineCount++;

          // limiter le nombre de lignes (éviter débordement mémoire)
          if (logLineCount > maxLogLines) {
            logDiv.removeChild(logDiv.firstChild);
            logLineCount--;
          }

          // autoscroll vers le bas
          logDiv.scrollTop = logDiv.scrollHeight;
        }

        function connect() {
          try {
            console.log(`[WS] Tentative de connexion à ${wsUrl} (attempt ${reconnectAttempts + 1})`);
            ws = new WebSocket(wsUrl);

            ws.onopen = function () {
              console.log("✓ WebSocket connecté à " + wsUrl);
              appendLogLine("[WS] ✓ connecté au serveur");
              reconnectAttempts = 0; // reset counter on success
              reconnectTimeout = 2000; // reset timeout
            };

            ws.onmessage = function (event) {
              appendLogLine(event.data);
            };

            ws.onclose = function () {
              console.log("[WS] Connexion fermée");
              appendLogLine("[WS] ✗ déconnecté, reconnexion dans " + Math.round(reconnectTimeout / 1000) + "s");
              
              // exponential backoff: augmente le timeout à chaque tentative
              reconnectAttempts++;
              reconnectTimeout = Math.min(2000 * Math.pow(1.5, reconnectAttempts), maxReconnectTimeout);
              console.log(`[WS] Prochaine tentative dans ${Math.round(reconnectTimeout / 1000)}s...`);
              
              setTimeout(connect, reconnectTimeout);
            };

            ws.onerror = function (err) {
              // Note: onerror ne fournit pas de détails sur l'erreur en WebSocket
              console.log("[WS] Erreur WebSocket (détails non disponibles en navigateur)");
              ws.close();
            };
          } catch (e) {
            console.error("[WS] Exception:", e.message);
            appendLogLine("[WS] ✗ exception: " + e.message);
            
            reconnectAttempts++;
            reconnectTimeout = Math.min(2000 * Math.pow(1.5, reconnectAttempts), maxReconnectTimeout);
            setTimeout(connect, reconnectTimeout);
          }
        }

        // Démarrer la connexion au chargement de la page
        if (document.readyState === "loading") {
          document.addEventListener("DOMContentLoaded", connect);
        } else {
          connect();
        }
      })();