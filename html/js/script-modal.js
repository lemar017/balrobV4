
        document.getElementById('openModal').addEventListener('click', function() {
            const iframe = document.getElementById('contentFrame');
            iframe.src = 'index2.html';
            document.getElementById('modalOverlay').style.display = 'flex';
        });

        window.addEventListener('message', function(e) {
            if (e.data === 'closeModal') {
                document.getElementById('modalOverlay').style.display = 'none';
                document.getElementById('contentFrame').src = 'about:blank';
            }
        });

        document.getElementById('modalOverlay').addEventListener('click', function(e) {
            if (e.target === this) {
                this.style.display = 'none';
                document.getElementById('contentFrame').src = 'about:blank';
            }
        });
