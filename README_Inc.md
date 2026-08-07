# RZ Tools — Incidenzimetro & Inclinometro RC

Strumento di misura per superfici mobili di aeromodelli RC, basato su **M5StickS3** (o M5StickC Plus2).  
Due applicazioni selezionabili dal menu iniziale: un **incidenzimetro** per la regolazione delle superfici di controllo e un **inclinometro** live con feedback audio.

---

## Hardware richiesto

| Componente | Dettaglio |
|---|---|
| Scheda | M5StickS3 (codice **K150**, chip ESP32-S3-PICO-1, IMU BMI270) |
| Alternativa | M5StickC Plus2 (K016-P2, IMU MPU6886) |
| Alimentazione | Batteria LiPo integrata 250 mAh, ricarica via USB-C |
| Involucro | Slitta/culla stampata in 3D con base piatta come piano di riferimento |

> **Nota sui tasti**: su questo esemplare i tasti risultano invertiti rispetto ai nomi di libreria.  
> DX (frontale) → `M5.BtnB` | BLU (sopra) → `M5.BtnA`  
> Se sul tuo esemplare i tasti rispondono al contrario, scambia `BtnA` e `BtnB` nelle righe del `loop()`.  
> Il tasto di accensione/spegnimento **non è utilizzabile**: premendolo si spegne la scheda.

---

## Dipendenze (Arduino IDE)

1. **Pacchetto schede ESP32**: aggiungi l'URL nelle preferenze:  
   `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`  
   poi installa **M5Stack** dal Gestore schede.

2. **Librerie** (Library Manager): `M5Unified` (include automaticamente `M5GFX`)

3. **Board**: `M5StickS3` (o `ESP32S3 Dev Module` con PSRAM abilitata).

---

## Flusso delle schermate

```
LOGO animato (logo RZ + aliante fluttuante)
  └─> MENU (DX scorre, BLU conferma)
        ├─> INCIDENZIMETRO
        │     INTRO (angolo animato)
        │       └─> COMANDI (legenda tasti)
        │             └─> IMPOSTA R (cerniera/bordo uscita)
        │                   └─> TARA (DX = azzera; BLU avanza SOLO dopo la tara)
        │                         └─> SX ALTO  -> click DX o BLU
        │                         └─> SX BASSO -> click DX o BLU
        │                         └─> (Premi BLU -> parte mobile DX)
        │                         └─> DX ALTO  -> click DX o BLU
        │                         └─> DX BASSO -> click DX o BLU
        │                               └─> MATRICE risultati
        │                                     BLU = menu | DX 2x = azzera
        │
        └─> INCLINOMETRO
              IMPOSTA R (distanza punto di misura)
                └─> TARA (DX = azzera; BLU avanza SOLO dopo la tara)
                      └─> LIVE (mm grande + gradi piccoli, audio crescente)
                            BLU = menu | DX = ri-tara
```

---

## Funzioni — Incidenzimetro

Misura l'escursione delle superfici mobili e confronta la simmetria SX/DX in mm e gradi.

**Sequenza di misura:**
1. Imposta **R** = distanza cerniera → bordo d'uscita (dove il costruttore misura la corsa)
2. **Tara** sul neutro: click DX. La scritta BLU rimane grigia ("prima fai tara!") finché non si esegue — se si preme BLU prima, parte un bip di avviso e non si avanza
3. Porta la superficie a fondo corsa, tienila ferma, click DX o BLU per registrare
4. Sequenza guidata: **SX alto → SX basso → (BLU per passare a DX) → DX alto → DX basso**
5. Matrice finale con valori in **mm** (grande) e **gradi** tra parentesi (piccolo), differenze difA e difB

**Visualizzazione durante la misura:** mm grande arancione + gradi grigio sotto

**Comandi:**

| Tasto | Schermata | Azione |
|---|---|---|
| click DX o BLU | misura fasi 1-4 | registra e avanza |
| click BLU | transizione SX→DX | passa alle misure DX |
| tieni DX | ovunque in misura | apre IMPOSTA R |
| click BLU | matrice finale | torna al menu |
| doppio click DX | matrice finale | azzera e torna al menu |

**Feedback sonoro:**
- Tono alla registrazione di ogni valore
- Verdetto simmetria: due note ascendenti = OK, tre toni gravi = fuori tolleranza (soglia 0.5°)
- Bip basso se BLU premuto prima della tara
- Allarme batteria scarica (<15%)

---

## Funzioni — Inclinometro

Misura l'inclinazione in tempo reale. Visualizza **mm in grande** (valore principale) e gradi in piccolo.

**Sequenza:**
1. Imposta **R** = distanza dal punto fisso al punto di misura
2. **Tara** sul neutro: click DX (stessa logica bloccante dell'incidenzimetro)
3. Schermata live: mm grande arancione + gradi grigio sotto

**Audio crescente:** più inclini → tono più acuto e tick più fitti. Sotto 1.5° silenzio.

**Comandi:**

| Tasto | Azione |
|---|---|
| click DX | ri-tara al volo |
| click BLU | torna al menu |

---

## Impostazione R

Incidenzimetro e inclinometro hanno R separata, salvata in NVS (resta dopo lo spegnimento).

| Tasto | Azione |
|---|---|
| click DX | +1 mm |
| doppio click DX | +10 mm |
| click BLU | −1 mm |
| doppio click BLU | −10 mm |
| tieni un tasto | conferma e prosegui |

---

## Note tecniche

**Asse di misura**: `atan2(ay, az)` — rotazione attorno all'asse X (lato lungo). Se risponde sull'asse sbagliato, cambia `readRawRoll()`:
```cpp
return atan2f(ax, az);   // asse corto
return -atan2f(ay, az);  // verso opposto
```
Usa `diagnostico_assi_m5.ino` per identificare l'asse corretto prima di usare il programma.

**Tara**: eseguita con media di 5 campioni freschi (`M5.Imu.update()` forzato). Effetto istantaneo a display. Obbligatoria: BLU è bloccato finché non viene eseguita.

**Filtro EMA**: `EMA_K = 0.90`. Alzare = più stabile/lento; abbassare = più reattivo.

**Conversione**: `mm = R × sin(angolo)`.

---

## File inclusi

| File | Descrizione |
|---|---|
| `incidenzimetro_m5_rz_v19.ino` | Programma principale (versione corrente) |
| `diagnostico_assi_m5.ino` | Sketch diagnostico per l'asse IMU |
| `test_lcd_m5.ino` | Sketch di test del display via seriale USB |

---

## Suggerimenti involucro/slitta

- Stampa la base con la **faccia inferiore sul piano di stampa** per massima planarità
- Aggiungi un'**aletta di registro** sul bordo d'uscita per posizionamento ripetibile
- Lascia apertura per il **connettore USB-C** (ricarica senza smontare)
- Non aprire l'involucro della M5StickS3: l'antenna WiFi è nel guscio

---

## Cronologia versioni principali

| Versione | Novità |
|---|---|
| **v19** | Tara bloccante (BLU disabilitato finché non eseguita); BLU torna al menu solo da matrice finale e inclinometro live |
| v18 | Inclinometro: mm grande, gradi piccoli; incidenzimetro: mm durante misura, matrice con gradi+mm; rimosso monitor seriale |
| v17 | Tara corretta: `taredRaw()` forza `update()` e media 5 campioni; `drawInclTara` mostra `liveDegI()` |
| v16 | Tara istantanea: reset di `smoothRad` elimina ritardo filtro EMA |
| v15 | Inclinometro con R e tara proprie; due R separate in NVS |
| v14 | Menu iniziale Incidenzimetro/Inclinometro; logo RZ animato |
| v10 | 4 misure guidate (SX/DX × alto/basso) con matrice finale |
| v9  | Logo RZ animato; azzera torna alla home |

---

## Licenza

Progetto personale — uso libero per scopi non commerciali.  
Sviluppato con il contributo di Claude (Anthropic).
