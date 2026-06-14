   // Configuration WiFi JavaScript
        class WiFiConfig {
            constructor() {
                this.form = document.getElementById('wifiForm');
                this.networkSelect = document.getElementById('wifiNetwork');
                this.refreshButton = document.getElementById('refreshNetworks');
                this.backButton = document.getElementById('backButton');
                this.statusMessage = document.getElementById('statusMessage');
                this.loading = document.getElementById('loading');
                
                this.initEventListeners();
                this.loadNetworks();
            }

            initEventListeners() {
                this.form.addEventListener('submit', (e) => this.handleSubmit(e));
                this.refreshButton.addEventListener('click', () => this.loadNetworks());
                this.backButton.addEventListener('click', () => this.goBack());
            }

            async loadNetworks() {
                this.networkSelect.innerHTML = '<option value="">Chargement...</option>';
                this.refreshButton.disabled = true;
                
                try {
                    const response = await fetch('/api/wifi/scan');
                    const networks = await response.json();
                    
                    this.networkSelect.innerHTML = '<option value="">Sélectionnez un réseau</option>';
                    
                    networks.forEach(network => {
                        const option = document.createElement('option');
                        option.value = network.ssid;
                        option.textContent = `${network.ssid} (${network.rssi} dBm)`;
                        this.networkSelect.appendChild(option);
                    });
                    
                } catch (error) {
                    console.error('Erreur lors du chargement des réseaux:', error);
                    this.networkSelect.innerHTML = '<option value="">Erreur de chargement</option>';
                    this.showMessage('Erreur lors du chargement des réseaux WiFi', 'error');
                } finally {
                    this.refreshButton.disabled = false;
                }
            }

            async handleSubmit(e) {
                e.preventDefault();
                
                const formData = new FormData(this.form);
                const config = {
                    mode: formData.get('connectionMode'),
                    ssid: formData.get('wifiNetwork'),
                    password: formData.get('password')
                };

                if (!config.ssid) {
                    this.showMessage('Veuillez sélectionner un réseau WiFi', 'error');
                    return;
                }

                this.showLoading(true);
                
                try {
                    const response = await fetch('/api/wifi/config', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                        },
                        body: JSON.stringify(config)
                    });

                    if (response.ok) {
                        this.showMessage('Configuration WiFi enregistrée avec succès !', 'success');
                        // Reset form après succès
                        setTimeout(() => {
                            this.form.reset();
                            document.getElementById('infrastructure').checked = true;
                        }, 1000);
                    } else {
                        throw new Error('Erreur lors de l\'enregistrement');
                    }
                    
                } catch (error) {
                    console.error('Erreur:', error);
                    this.showMessage('Erreur lors de l\'enregistrement de la configuration', 'error');
                } finally {
                    this.showLoading(false);
                }
            }

            showLoading(show) {
                this.loading.style.display = show ? 'block' : 'none';
                this.form.style.display = show ? 'none' : 'block';
            }

            showMessage(message, type) {
                this.statusMessage.textContent = message;
                this.statusMessage.className = `status-message status-${type}`;
                this.statusMessage.style.display = 'block';
                
                // Masquer le message après 5 secondes
                setTimeout(() => {
                    this.statusMessage.style.display = 'none';
                }, 5000);
            }

            goBack() {
                window.location.href = 'index.html';
            }
        }

        // Initialiser l'application
        document.addEventListener('DOMContentLoaded', () => {
            new WiFiConfig();
        });

