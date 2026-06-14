ws.onmessage = e => {
  try {
    const msg = JSON.parse(e.data);

    if (msg.event === 'serial') {
      // Message venant de Serial/LOG() sur l'ESP32
      addLog('[ESP32] ' + msg.msg, '');
    }
    else if (msg.event === 'saved')  { ... }
    else if (msg.event === 'error')  { ... }
    // etc.
  } catch(_) {
    addLog('[WS] ' + e.data, '');
  }
};
```

---

## Flux complet, pas à pas
```
1. Code C++ appelle :
   LOG("[IMU] angle=89.7° err=0.3°\n")

2. SerialOverWS::write() reçoit chaque caractère
   → accumule dans _buf : "[IMU] angle=89.7° err=0.3°"
   → '\n' détecté → flush()

3. flush() sérialise en JSON :
   {"event":"serial","msg":"[IMU] angle=89.7° err=0.3°"}
   → ws.textAll() → envoyé à tous les navigateurs connectés

4. js ws.onmessage reçoit le JSON
   → msg.event === 'serial'
   → addLog('[ESP32] [IMU] angle=89.7° err=0.3°', '')

5. addLog() crée une <div class="term-line"> horodatée
   → insérée dans #terminal → scrollTop mis à jour
   
   
   
   Pourquoi passer par une classe Print ?
Serial sur Arduino/ESP32 est lui-même une classe qui hérite de Print. La classe Print définit une interface avec une seule méthode abstraite : write(uint8_t). Tout le reste (printf, println, print) est construit par-dessus. En héritant de Print et en redéfinissant uniquement write(), on intercepte toutes les formes d'écriture sans modifier le reste du code.