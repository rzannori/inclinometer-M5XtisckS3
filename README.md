# RZ Tools — Confronto Escursioni, Goniometro & Log Volo

Strumento per aeromodelli RC basato su **M5StickS3** (o M5StickC Plus2).
Tre applicazioni selezionabili dal menu iniziale:

1. **Confronto Escursioni** — misura l'escursione delle superfici mobili e confronta la simmetria SX/DX
2. **Goniometro** — inclinazione live in mm e gradi, con feedback audio
3. **Log volo (GPS)** — grafico scorrevole di vario, quota o velocità letti da un log NMEA

Ultima versione con tre strumenti: **`old/incidenzimetro_m5_rz_v23/`** (documentata qui sotto).

> **v25 — RZTools, versione di sviluppo corrente** (`RZTools_v25/RZTools_v25.ino`).
> Menu con due sottoprogrammi: **Goniometro** (come v24.2) e **Inclinometro** (confronto escursioni riscritto).
> Il Log volo è temporaneamente rimosso. Non richiede i file `dati_test_nmea*.h` né la libreria SD.
> Vedi [RZTools v25](#rztools-v25).

---

## RZTools v25

```
LOGO → MENU ─┬─ Goniometro  : IMPOSTA R → MONTA → TARA → LIVE
             └─ Inclinometro: IMPOSTA R → MONTA SX → TARA → REGISTRA SX
                              → MONTA DX → VERIFICA ZERO [→ ALLARME] → REGISTRA DX → RISULTATO
```

- **Menu**: DX ("Cambia") scorre, BLU ("ok") conferma
- **Imposta R** (unica per entrambi i programmi): DX +1 / 2 click +10, BLU −1 / 2 click −10, **tieni DX = conferma**, **tieni BLU = menu**
- **Goniometro**: identico alla v24.2 (vedi [Goniometro v24.x](#goniometro-v24x))

### Inclinometro (confronto escursioni)

1. **Imposta R**, poi **posiziona lo stick sulla superficie SX al neutro** e premi **Avvia Tara** (tara stabilizzata come nel Goniometro)
2. **Registrazione SX**: muovi la superficie a fondo corsa in su e in giù. A sinistra la misura continua (mm grande, gradi piccoli); a destra, separati da una linea verticale, **Max<sub>S</sub>** (in alto) e **min<sub>S</sub>** (in basso) in mm con i gradi più piccoli sotto
3. **Chiusura automatica**: quando sia Max sia min superano 1 mm parte il conteggio; ogni nuovo Max/min che supera il precedente di almeno 0.5 mm lo fa ripartire. Dopo **2 s senza variazioni** la registrazione si chiude (in basso a sinistra: "fine tra x.x s"). **DX ("Fine")** chiude a mano in qualsiasi momento (utile per superfici che si muovono in una sola direzione, es. flap)
4. **Sposta lo stick sulla superficie DX al neutro** e premi **Avvia**: lo stick attende di essere fermo e **verifica lo zero** (la tara resta quella della SX). Se lo zero DX differisce di oltre **0.5 mm** compare un **allarme** con la differenza in mm e gradi: DX = continua comunque, BLU = riposiziona
5. **Registrazione DX**: come la SX, con **Max<sub>D</sub>** e **min<sub>D</sub>**
6. **Risultato**: tabella SX / DX / differenza per Max e min (mm, gradi piccoli sotto), differenze in verde se entro tolleranza (0.5°) e in rosso se fuori, differenza di zero DX, verdetto "SIMMETRIA OK" / "FUORI TOLLERANZA" con verdetto sonoro. DX = nuova misura, BLU = menu

| Schermata | DX (freccia in alto) | BLU (freccia a destra) |
|---|---|---|
| Monta SX | Avvia Tara | back → Imposta R |
| Registrazione | Fine | back → rimonta la superficie |
| Monta DX | Avvia (verifica zero) | back → ricomincia dalla SX |
| Allarme zero | Continua | back → riposiziona DX |
| Risultato | Nuova misura | menu |

Parametri in testa allo sketch: `ARM_MM` (1.0), `VAR_MM` (0.5), `NOVAR_MS` (2000), `ZERO_TOL_MM` (0.5), `SYM_TOL_DEG` (0.5).

---

## Hardware richiesto

| Componente | Dettaglio |
|---|---|
| Scheda | M5StickS3 (codice **K150**, chip ESP32-S3-PICO-1, IMU BMI270) |
| Alternativa | M5StickC Plus2 (K016-P2, IMU MPU6886) |
| Alimentazione | Batteria LiPo integrata, ricarica via USB-C |
| Involucro | Slitta/culla stampata in 3D con base piatta come piano di riferimento |
| Modulo SD (opzionale) | Solo per il Log volo da scheda SD: modulo SD SPI sull'Hat2-Bus dello StickS3 |

**Collegamento modulo SD (SPI):**

| Segnale | Pin |
|---|---|
| CS | G1 |
| SCK | G6 |
| MOSI | G7 |
| MISO | G8 |

> **Nota sui tasti**: su questo esemplare i tasti risultano invertiti rispetto ai nomi di libreria.
> DX (frontale) → `M5.BtnB` | BLU (sopra) → `M5.BtnA`
> Se sul tuo esemplare i tasti rispondono al contrario, scambia `BtnA` e `BtnB` nel blocco di rilevamento tasti all'inizio del `loop()`.
> Il tasto di accensione/spegnimento **non è utilizzabile**: premendolo si spegne la scheda.
>
> **Click e pressione prolungata**: "click" = pressione breve (evento al rilascio); "tieni" = pressione oltre 600 ms (`HOLD_MS`); "doppio click" = due click entro 300 ms (`DBL_MS`).

---

## Dipendenze e caricamento (Arduino IDE)

1. **Pacchetto schede**: aggiungi l'URL nelle preferenze:
   `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`
   poi installa **M5Stack** dal Gestore schede.
2. **Librerie** (Library Manager): `M5Unified` (include automaticamente `M5GFX`).
   `SPI`, `SD` e `Preferences` sono già incluse nel core ESP32.
3. **Board**: `M5StickS3` (o `ESP32S3 Dev Module` con PSRAM abilitata).

### File dati di test (obbligatori per compilare)

Lo sketch v23 include due header con i dati di test del Log volo:

| File | Contenuto |
|---|---|
| `dati_test_nmea.h` | Estratto da un log reale (`TEST_NMEA_DATA`, 117 campioni a 0.1 s), per verificare il parsing |
| `dati_test_nmea_sintetici.h` | Dati sintetici (`TEST_NMEA_DATA_SYNTH`, 220 campioni): vario −10/+10 m/s, quota 0–400 m, velocità 0–150 km/h; profilo salita → termiche → planata/atterraggio |

Entrambi i file sono inclusi nel repository e **sono necessari per compilare**.

L'Arduino IDE richiede che la cartella dello sketch abbia **lo stesso nome** del file `.ino` e che i `.h` stiano **nella stessa cartella**:

Nel repository la cartella `old/incidenzimetro_m5_rz_v23/` è già organizzata così: basta copiarla e
aprire il `.ino` dall'IDE: i due `.h` compaiono come schede aggiuntive in alto, segno che sono stati riconosciuti

---

## Flusso delle schermate

```
LOGO animato (logo RZ fluttuante) — un tasto qualsiasi
  └─> MENU (DX scorre, BLU conferma)
        ├─> CONFRONTO ESCURSIONI
        │     INTRO (angolo animato) — un tasto
        │       └─> FUNZIONI TASTI — un tasto
        │             └─> IMPOSTA DISTANZA R (bordo d'uscita – cerniera)
        │                   └─> TARA (DX = azzera; BLU avanza SOLO dopo la tara)
        │                         └─> SX ALTO  -> click DX o BLU
        │                         └─> SX BASSO -> click DX o BLU
        │                         └─> "Parte mobile SX OK" -> premi BLU
        │                         └─> DX ALTO  -> click DX o BLU
        │                         └─> DX BASSO -> click DX o BLU
        │                               └─> MATRICE risultati
        │                                     BLU = menu | DX 2x = azzera e menu
        │
        ├─> GONIOMETRO
        │     COMANDI (funzione tasti) — un tasto
        │       └─> IMPOSTA R (bordo d'uscita – cerniera)
        │             └─> TARA (DX = azzera; BLU avanza SOLO dopo la tara)
        │                   └─> LIVE (mm grande + gradi piccoli, audio crescente)
        │                         BLU = menu | DX = ri-tara
        │
        └─> LOG VOLO (GPS)
              SORGENTE DATI (DX cambia, BLU conferma)
                └─> COMANDI — un tasto
                      └─> GRAFICO scorrevole
                            DX = cambia variabile | BLU = menu
```

---

## Confronto Escursioni

Misura l'escursione delle superfici mobili e confronta la simmetria SX/DX in mm e gradi.

**Sequenza di misura:**
1. Imposta **R** = distanza tra bordo d'uscita e cerniera
2. **Tara** sul neutro: click DX. Dopo la tara compare il riquadro verde "Pronto per la misura". La scritta BLU resta grigia ("prima fai tara!") finché la tara non è eseguita: se si preme BLU prima, parte un bip basso e non si avanza
3. Porta la superficie a fondo corsa, tienila ferma, click DX o BLU per registrare
4. Sequenza guidata: **SX alto → SX basso → (BLU per passare a DX) → DX alto → DX basso** (indicatore di passo `n/4` in alto a destra)
5. Matrice finale con valori in **mm** (grande) e **gradi** tra parentesi (piccolo), più le differenze di simmetria **difA** (alto) e **difB** (basso) in mm

**Visualizzazione durante la misura:** mm grande arancione + gradi grigio sotto.

**Comandi:**

| Tasto | Schermata | Azione |
|---|---|---|
| click DX o BLU | letture 1-4 | registra e avanza |
| click BLU | passaggio SX→DX | passa alle misure DX |
| tieni DX | letture / matrice | apre IMPOSTA R, poi rientra in misura |
| click BLU | matrice finale | torna al menu |
| doppio click DX | matrice finale | azzera e torna al menu |

**Feedback sonoro:**
- Tono alla registrazione di ogni valore
- Verdetto simmetria a fine sequenza: due note ascendenti = OK, tre toni gravi = fuori tolleranza (soglia **0.5°** sulla differenza dei valori assoluti, sia in alto sia in basso)
- Bip basso se BLU premuto prima della tara

---

## Goniometro

Misura l'inclinazione in tempo reale. Visualizza **mm in grande** (valore principale), gradi in piccolo e R in basso a sinistra.

**Sequenza:**
1. Pagina **comandi** (funzione tasti) — un tasto qualsiasi per proseguire
2. Imposta **R** = distanza tra bordo d'uscita e cerniera (R separata da quella del Confronto Escursioni)
3. **Tara** sul neutro: click DX (stessa logica bloccante del Confronto Escursioni)
4. Schermata live: mm grande arancione + gradi grigio sotto

**Audio crescente:** più inclini (in + o in −) → tono più acuto e tick più fitti. Sotto 1.5° silenzio.

**Comandi (schermata live):**

| Tasto | Azione |
|---|---|
| click DX | ri-tara al volo |
| click BLU | torna al menu |

> Nota: la pagina comandi del Goniometro mostra "tieni DX = imposta dist.", ma nella v23 questa funzione non è implementata nella schermata live. Per cambiare R si torna al menu e si rientra nel Goniometro.

### Goniometro v24.x

Flusso:
```
LOGO → IMPOSTA R → MONTA (Avvia Tara = DX) → TARA IN CORSO → LIVE
          ↑                                                 │
          └──────────────────── BLU (back) ─────────────────┤
                         TARA IN CORSO ←── DX ──────────────┘
```

1. **Imposta R** con lo stick in mano (DX +1 / 2 click +10, BLU −1 / 2 click −10, **tieni DX o BLU** = conferma)
2. **Monta** lo stick sulla superficie al neutro e premi **Avvia Tara** (tasto DX)
3. **Tara stabilizzata**: togli il dito; lo stick attende di restare fermo (oscillazione < 0.15° per 1 s, barra di avanzamento), poi esegue la tara su 20 campioni, suona due note ascendenti e passa da solo alla misura. Se non si stabilizza entro 8 s emette un bip basso e riprova
4. **Live**: mm grande + gradi piccoli, audio crescente. **DX** = rifai la tara (stessa logica), **BLU** = torna a impostare R

| Schermata | click DX | click BLU |
|---|---|---|
| Monta | avvia la tara | torna a R |
| Tara in corso | ricomincia l'attesa | annulla, torna a R |
| Live | rifai la tara | imposta R |

**Frecce dei tasti (v24.1):** in ogni schermata, tranne il logo, due frecce indicano dove si trovano i tasti fisici:
- **DX**: in alto al centro, punta verso l'alto, dello stesso colore della scritta che spiega il tasto DX in quella schermata
- **BLU**: a destra, centrata in verticale, punta verso destra, blu come la scritta del tasto BLU

**v24.2:** dopo il logo si va direttamente a **Imposta R** (la pagina comandi è stata tolta). Nella schermata di montaggio la freccia DX è etichettata **"Avvia Tara"**; accanto alla freccia BLU compare **"back"** in verticale nelle schermate in cui BLU torna indietro (montaggio, tara in corso, misura).

Parametri regolabili in testa allo sketch: `STAB_TOL_DEG` (0.15°), `STAB_MS` (1000 ms), `TARA_TIMEOUT_MS` (8000 ms), `TARA_SAMPLES` (20).

---

## Log volo (GPS)

Legge un log NMEA esteso (frasi `$SMGPS` / `$SMGPS2`, tipiche dei logger GPS/vario per alianti RC) e lo riproduce come grafico scorrevole.

**Sorgenti dati selezionabili:**

| Voce | Sorgente |
|---|---|
| Test sintetico | `dati_test_nmea_sintetici.h` (default; funziona senza SD) |
| Test reale (file) | `dati_test_nmea.h` (estratto da un log vero) |
| Scheda SD | file **`/log.nmea`** nella radice della SD |

La SD viene montata **una sola volta all'avvio**: se non è inserita o non risponde, nella schermata della sorgente compare "SD non rilevata!" e per usarla bisogna riavviare la scheda.

**Grafico:**
- Variabili (si cambiano con click DX): **VARIO m/s** (scala −10…+10, con linea dello zero), **QUOTA m** (0…400), **VELOCITÀ km/h** (0…150); i valori fuori scala vengono tagliati al bordo
- Valore corrente al centro in alto, scala min/max a sinistra del grafico
- In alto a destra: **tempo di log trascorso** dall'inizio della riproduzione (campioni × intervallo: 0.5 s per i dati sintetici, 0.1 s per test reale e SD)
- Finestra di 198 campioni, un nuovo campione ogni 120 ms (`LOG_PLAY_MS`); a fine dati la riproduzione ricomincia da capo

**Formato atteso** (le altre frasi NMEA vengono ignorate):
```
$SMGPS,<quota> m,...                                         -> quota (2° campo)
$SMGPS2,X,Y,Z,ENL,us,<vel> kmh,<press>,<press>,<vario> m/s*CS -> velocità (7° campo) e vario (10° campo)
```
Un campione viene formato da una coppia `$SMGPS` + `$SMGPS2` consecutiva.

**Comandi:**

| Tasto | Schermata | Azione |
|---|---|---|
| click DX | sorgente dati | cambia sorgente |
| click BLU | sorgente dati | conferma |
| un tasto | comandi | avvia il grafico |
| click DX | grafico | cambia variabile (vario / quota / velocità) |
| click BLU | grafico | torna al menu |

---

## Impostazione R

Confronto Escursioni e Goniometro hanno R separate, salvate in NVS (restano dopo lo spegnimento). Valore minimo 1 mm.

| Tasto | Confronto Escursioni | Goniometro |
|---|---|---|
| click DX | +1 mm | +1 mm |
| doppio click DX | +10 mm | +10 mm |
| click BLU | −1 mm | −1 mm |
| doppio click BLU | −10 mm | −10 mm |
| conferma | **tieni DX o BLU** | **tieni BLU** |

---

## Funzioni generali

- **Allarme batteria scarica**: controllo ogni 30 s, doppio bip se la carica scende sotto il 15%
- **IMU non trovato**: all'avvio schermo rosso con messaggio e toni di errore; il programma si ferma
- **Audio**: abilitato di default; lo stato è letto da NVS (chiave `snd`)

---

## Note tecniche

**Asse di misura**: `atan2(ay, az)` — rotazione attorno all'asse X (lato lungo). Se risponde sull'asse sbagliato, cambia `readRawRoll()`:
```cpp
return atan2f(ax, az);   // asse corto
return -atan2f(ay, az);  // verso opposto
```

**Tara**: eseguita con media di 5 campioni freschi (`M5.Imu.update()` forzato). Effetto istantaneo a display (reset del filtro). Obbligatoria: BLU è bloccato finché non viene eseguita. Confronto Escursioni e Goniometro hanno tare separate.

**Filtro EMA**: `EMA_K = 0.90`. Alzare = più stabile/lento; abbassare = più reattivo.

**Conversione**: `mm = R × sin(angolo)`.

**Parametri principali** (in testa allo sketch):

| Costante | Valore | Significato |
|---|---|---|
| `SYM_TOL_DEG` | 0.5 | tolleranza simmetria (°) |
| `HOLD_MS` | 600 | soglia pressione prolungata (ms) |
| `DBL_MS` | 300 | finestra doppio click (ms) |
| `BATT_LOW_PCT` | 15 | soglia batteria scarica (%) |
| `LOG_PLAY_MS` | 120 | velocità di scorrimento del grafico (ms/campione) |
| `SD_LOG_FILENAME` | `/log.nmea` | file letto dalla SD |

---

## Struttura del repository

Ogni sketch sta in una cartella con **lo stesso nome** del file `.ino`, come richiesto dall'Arduino IDE: basta copiare la cartella e aprire il `.ino`.

```
README.md
RZTools_v25/
  RZTools_v25.ino                 versione corrente: menu RZTools con Goniometro e Inclinometro
old/                              versioni precedenti (archivio)
  incidenzimetro_m5_rz_v24_2/     solo Goniometro: "Avvia Tara", "back", R subito dopo il logo
  incidenzimetro_m5_rz_v24_1/     v24 + frecce che indicano i tasti
  incidenzimetro_m5_rz_v24/       solo Goniometro con tara stabilizzata
  incidenzimetro_m5_rz_v23/       tre strumenti (Confronto Escursioni, Goniometro, Log volo)
    dati_test_nmea.h              dati di test reali per il Log volo (richiesti dalla v23)
    dati_test_nmea_sintetici.h    dati di test sintetici per il Log volo (richiesti dalla v23)
  incidenzimetro_m5_rz_v20/       due strumenti (Incidenzimetro, Inclinometro)
  incidenzimetro_m5_rz_v13/       versione storica (solo incidenzimetro)
```

Quando esce una nuova versione, la precedente viene spostata in `old/`.

---

## Suggerimenti involucro/slitta

- Stampa la base con la **faccia inferiore sul piano di stampa** per massima planarità
- Aggiungi un'**aletta di registro** sul bordo d'uscita per posizionamento ripetibile
- Lascia apertura per il **connettore USB-C** (ricarica senza smontare) e, se usi il Log volo da SD, per l'**Hat2-Bus**
- Non aprire l'involucro della M5StickS3: l'antenna WiFi è nel guscio

---

## Cronologia versioni principali

Numerazione: **decimale** (v24.1, v24.2…) per modifiche minori, **intero** (v25…) per modifiche importanti. Nel nome del file il punto diventa `_` (es. `v24_1`).

| Versione | Novità |
|---|---|
| **v25** | RZTools: menu con Goniometro e Inclinometro; Inclinometro (confronto escursioni) riscritto: tara unica sulla SX, registrazione automatica Max/min con chiusura dopo 2 s senza variazioni, verifica zero DX con allarme, riepilogo con verdetto; R unica per entrambi; tieni BLU in Imposta R = menu |
| v24.2 | Dopo il logo direttamente a Imposta R (tolta la pagina comandi); titolo "Imposta Distanza cerniera bordo di uscita (mm)"; montaggio: etichetta "Avvia Tara" sotto la freccia DX, istruzioni uniformi "Poi premi Avvia Tara"; scritta "back" verticale accanto alla freccia BLU; valori della misura spostati leggermente a sinistra |
| v24.1 | Frecce che indicano la posizione dei tasti: DX in alto al centro (colore della scritta DX), BLU a destra al centro (blu); titoli abbassati per far posto alla freccia DX |
| v24 | Solo Goniometro (altri strumenti rimossi temporaneamente); tara ritardata e stabilizzata con passaggio automatico alla misura; BLU in misura = imposta R; conferma R con DX o BLU |
| v23 | tre strumenti (Confronto Escursioni, Goniometro, Log volo) |
| v22 | Strumenti rinominati (Incidenzimetro → Confronto Escursioni, Inclinometro → Goniometro); pagina "funzione tasti" anche per il Goniometro; testi delle pagine R/tara aggiornati; nel Log volo in alto a destra il tempo di riproduzione trascorso al posto della sorgente dati |
| v21 | Log volo (GPS/vario): lettura NMEA `$SMGPS`/`$SMGPS2` da dati di test o SD, grafico scorrevole di vario/quota/velocità |
| v20 | Due strumenti (Incidenzimetro, Inclinometro) |
| v19 | Tara bloccante (BLU disabilitato finché non eseguita); BLU torna al menu solo da matrice finale e inclinometro live |
| v18 | Inclinometro: mm grande, gradi piccoli; incidenzimetro: mm durante misura, matrice con gradi+mm; rimosso monitor seriale |
| v17 | Tara corretta: `taredRaw()` forza `update()` e media 5 campioni |
| v16 | Tara istantanea: reset di `smoothRad` elimina ritardo filtro EMA |
| v15 | Inclinometro con R e tara proprie; due R separate in NVS |
| v14 | Menu iniziale Incidenzimetro/Inclinometro; logo RZ animato |
| v13 | Matrice finale con valori più grandi e differenze difA/difB sulla stessa riga |
| v10 | 4 misure guidate (SX/DX × alto/basso) con matrice finale |
| v9  | Logo RZ animato; azzera torna alla home |

---

## Licenza

Progetto personale — uso libero per scopi non commerciali.
Sviluppato con il contributo di Claude (Anthropic).
