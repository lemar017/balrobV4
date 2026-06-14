        const joystickball = document.querySelector('.joystickball');

        const zone2 = document.getElementById('zone2');
        const posX = document.getElementById('posX');
        const posY = document.getElementById('posY');

        let isDragging = false;
        let startX = 0;
        let startY = 0;
        let initialX = 0;
        let initialY = 0;

        // Position initiale au centre
        let currentX = 0;
        let currentY = 0;

        function updatePosition(x, y) {
            // Limiter le déplacement à +/- 100px
            const limitedX = Math.max(-100, Math.min(100, x));
            const limitedY = Math.max(-100, Math.min(100, y));
            
            currentX = limitedX;
            currentY = limitedY;
			console.log("updatePosition");
            
            // Appliquer la transformation
            joystickball.style.transform = `translate(calc(-50% + ${currentX}px), calc(-50% + ${currentY}px))`;
            
            // Mettre à jour les champs de texte
            posX.value = currentX;
            posY.value = - currentY;
            
            // Envoyer les coordonnées au programme C++
            sendCoordinatesToCpp(currentX, - currentY);
        }

        function sendCoordinatesToCpp(x, y) {
            // Envoi vers ESP32 (remplacez l'IP par celle de votre ESP32)
			
            fetch("/updateXYJoystick", {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    valX: x,
                    valY: y,
                    timestamp: Date.now()
                })//modif -y
            })
            .then(response => {
                if (!response.ok) {
                    console.warn('Erreur lors de l\'envoi des coordonnées:', response.status);
                }
                return response.json();
            })
            .then(data => {
                console.log('Réponse ESP32:', data);
            })
            .catch(error => {
                console.warn('Impossible de contacter l\'ESP32:', error);
            });
        }

        // Optionnel: Connexion WebSocket pour communication en temps réel
        /*
        let websocket = null;
        
        function connectWebSocket() {
            try {
                websocket = new WebSocket('ws://localhost:8081');
                
                websocket.onopen = function(event) {
                    console.log('Connexion WebSocket établie');
                };
                
                websocket.onmessage = function(event) {
                    const data = JSON.parse(event.data);
                    console.log('Réponse du serveur C++:', data);
                };
                
                websocket.onclose = function(event) {
                    console.log('Connexion WebSocket fermée');
                };
                
                websocket.onerror = function(error) {
                    console.error('Erreur WebSocket:', error);
                };
            } catch (error) {
                console.error('Impossible de se connecter au WebSocket:', error);
            }
        }
        
        // Décommentez la ligne suivante pour activer WebSocket
        // connectWebSocket();
        */

        // Événements pour la souris
            joystickball.addEventListener('mousedown', (e) => {
            isDragging = true;
            startX = e.clientX;
            startY = e.clientY;
            initialX = currentX;
            initialY = currentY;
            joystickball.style.cursor = 'grabbing';
            console.log("mousedown");  // ← Ajoute ça pour vérifier
            e.preventDefault();
        });

        document.addEventListener('mousemove', (e) => {
            if (!isDragging) return;
            
            const deltaX = e.clientX - startX;
            const deltaY = e.clientY - startY;
            console.log("mousemove");
            updatePosition(initialX + deltaX, initialY + deltaY);
        });

        document.addEventListener('mouseup', () => {
            if (isDragging) {
                isDragging = false;
                joystickball.style.cursor = 'move';
                
                // Retour automatique en position 0,0
                returnToCenter();
            }
        });

        // Événements pour le tactile
            joystickball.addEventListener('touchstart', (e) => {
            isDragging = true;
            const touch = e.touches[0];
            startX = touch.clientX;
            startY = touch.clientY;
            initialX = currentX;
            initialY = currentY;
            e.preventDefault();
        });

        document.addEventListener('touchmove', (e) => {
            if (!isDragging) return;
            
            const touch = e.touches[0];
            const deltaX = touch.clientX - startX;
            const deltaY = touch.clientY - startY;
            
            updatePosition(initialX + deltaX, initialY + deltaY);
            e.preventDefault();
        });

        document.addEventListener('touchend', () => {
            if (isDragging) {
                isDragging = false;
                
                // Retour automatique en position 0,0
                returnToCenter();
            }
        });

        // Fonction pour retourner au centre avec animation
        function returnToCenter() {
            const startX = currentX;
            const startY = currentY;
            const duration = 500; // 500ms pour l'animation
            const startTime = Date.now();
            
            function animate() {
                const elapsed = Date.now() - startTime;
                const progress = Math.min(elapsed / duration, 1);
                
                // Fonction d'easing pour un mouvement fluide
                const easeOut = 1 - Math.pow(1 - progress, 3);
                
                const newX = startX + (0 - startX) * easeOut;
                const newY = startY + (0 - startY) * easeOut;
                
                updatePosition(newX, newY);
                
                if (progress < 1) {
                    requestAnimationFrame(animate);
                }
            }
            
            animate();
        }

        // Initialiser l'affichage
        updatePosition(0, 0);