document.addEventListener('DOMContentLoaded', () => {
  const infraRadio = document.getElementById('infrastructure');
  const adhocRadio = document.getElementById('adhoc');
  const wifiSelect = document.getElementById('wifiNetwork');
  const newScanBtn = document.getElementById('newScan');
  const wifiForm = document.getElementById('wifiForm');
  const statusMessage = document.getElementById('statusMessage');
  const passwordInput = document.getElementById('password');

  function setStatus(msg, ok = true) {
    statusMessage.textContent = msg;
    statusMessage.style.color = ok ? 'green' : 'crimson';
    statusMessage.style.display = 'block';
    setTimeout(() => { statusMessage.style.display = 'none'; }, 4000);
  }

  function enableInfrastructure(enabled) {
    wifiSelect.disabled = !enabled;
    newScanBtn.disabled = !enabled;
    passwordInput.disabled = !enabled;
  }

  infraRadio.addEventListener('change', () => enableInfrastructure(true));
  adhocRadio.addEventListener('change', () => enableInfrastructure(false));

  async function scanNetworks() {
    wifiSelect.innerHTML = '<option value="">Scanning...</option>';
    // Try modern endpoint first
    try {
      const res = await fetch('/scan');
      if (res.ok) {
        const data = await res.json();
        populateNetworks(data);
        return;
      } else {
        const text = await res.text();
        console.warn('/scan returned', res.status, text);
        // fall through to legacy flow
      }
    } catch (e) {
      console.warn('/scan fetch failed', e);
    }

    // Legacy fallback: /start-scan then poll /get-scan-result
    try {
      const start = await fetch('/start-scan');
      if (!start.ok) {
        const t = await start.text();
        throw new Error('/start-scan failed: ' + start.status + ' ' + t);
      }

      setStatus('Scan started (legacy flow), waiting for results...');
      // poll for results
      const poll = async () => {
        try {
          const r = await fetch('/get-scan-result');
          if (!r.ok) {
            const t = await r.text();
            console.warn('/get-scan-result returned', r.status, t);
            // Try again later
            setTimeout(poll, 1500);
            return;
          }
          const data = await r.json();
          if (Array.isArray(data)) {
            populateNetworks(data);
          } else if (data && data.status === 'scan_in_progress') {
            // still scanning
            setTimeout(poll, 1500);
          } else {
            // unexpected response
            console.warn('/get-scan-result unexpected', data);
            setStatus('Scan returned unexpected result', false);
            wifiSelect.innerHTML = '<option value="">Scan error</option>';
          }
        } catch (err) {
          console.error('Polling scan error', err);
          setTimeout(poll, 1500);
        }
      };
      setTimeout(poll, 1000);
    } catch (err) {
      console.error('Legacy scan start failed', err);
      wifiSelect.innerHTML = '<option value="">Scan error</option>';
      setStatus('Scan error: ' + err.message, false);
    }
  }

  function populateNetworks(data) {
    wifiSelect.innerHTML = '';
    if (Array.isArray(data) && data.length) {
      data.sort((a,b) => (b.rssi || 0) - (a.rssi || 0));
      data.forEach(net => {
        const opt = document.createElement('option');
        opt.value = net.ssid || '';
        const enc = net.encryption || net.auth || 'open';
        opt.textContent = `${net.ssid} (${enc}, ${net.rssi} dBm)`;
        wifiSelect.appendChild(opt);
      });
    } else {
      const opt = document.createElement('option');
      opt.value = '';
      opt.textContent = 'No networks found';
      wifiSelect.appendChild(opt);
    }
    setStatus('Scan complete. ' + (Array.isArray(data) ? data.length : 0) + ' networks found');
  }

  newScanBtn.addEventListener('click', scanNetworks);

  // Load current config and perform initial scan if in STA
  async function loadCurrent() {
    // Try /wificurrent then fallback to /get-wifi-config for older pages
    let cfg = null;
    try {
      const res = await fetch('/wificurrent');
      if (res.ok) cfg = await res.json();
      else console.warn('/wificurrent returned', res.status, await res.text());
    } catch (e) {
      console.warn('fetch /wificurrent failed', e);
    }

    if (!cfg) {
      try {
        const r2 = await fetch('/get-wifi-config');
        if (r2.ok) cfg = await r2.json();
        else console.warn('/get-wifi-config returned', r2.status, await r2.text());
      } catch (e) {
        console.warn('fetch /get-wifi-config failed', e);
      }
    }

    if (!cfg) {
      // no config, try scanning anyway
      scanNetworks();
      return;
    }

    // cfg loaded
    
    if (cfg.mode && cfg.mode === 'AP') {
      adhocRadio.checked = true;
      enableInfrastructure(false);
    } else {
      infraRadio.checked = true;
      enableInfrastructure(true);
      await scanNetworks();
      // select current SSID if present
      if (cfg.ssid) {
        let found = Array.from(wifiSelect.options).some(o => o.value === cfg.ssid);
        if (!found) {
          const o = document.createElement('option');
          o.value = cfg.ssid;
          o.textContent = cfg.ssid + ' (current)';
          wifiSelect.insertBefore(o, wifiSelect.firstChild);
        }
        wifiSelect.value = cfg.ssid;
      }
    }
  }
  wifiForm.addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const mode = infraRadio.checked ? 'STA' : 'AP';
    const ssid = wifiSelect.value || '';
    const password = passwordInput.value || '';

    // basic validation
    if (mode === 'STA' && !ssid) {
      setStatus('Select a WiFi network first', false);
      return;
    }

    const body = { mode: mode === 'STA' ? 'STA' : 'AP', ssid: ssid, password: password };

    try {
      const res = await fetch('/wificonfig', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body)
      });
      if (!res.ok) {
        const text = await res.text();
        setStatus('Save failed: ' + res.status + ' ' + text, false);
      } else {
        setStatus('Configuration saved');
      }
    } catch (e) {
      console.error('wificonfig post error', e);
      setStatus('Error saving config: ' + e.message, false);
    }
  });

  // Start
  loadCurrent();
});
