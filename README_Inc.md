# RZ Tools — Confronto Escursioni, Goniometro & Log Volo

Strumento per aeromodelli RC basato su **M5StickS3** (o M5StickC Plus2).
Tre applicazioni selezionabili dal menu iniziale:

1. **Confronto Escursioni** — misura l'escursione delle superfici mobili e confronta la simmetria SX/DX
2. **Goniometro** — inclinazione live in mm e gradi, con feedback audio
3. **Log volo (GPS)** — grafico scorrevole di vario, quota o velocità letti da un log NMEA

Versione corrente: **`incidenzimetro_m5_rz_v23.ino`**

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
| `dati_test_nmea.h` | Estratto da un log reale (`TEST_NMEA_DATA`), per verificare il parsing |
| `dati_test_nmea_sintetici.h` | Dati sintetici (`TEST_NMEA_DATA_SYNTH`): vario −10/+10 m/s, quota 0–400 m, velocità 0–150 km/h |

> ⚠️ **Questi due file non sono ancora presenti nel repository**: senza di essi lo sketch non compila.

L'Arduino IDE richiede che la cartella dello sketch abbia **lo stesso nome** del file `.ino` e che i `.h` stiano **nella stessa cartella**:

1. crea una cartella chiamata esattamente `incidenzimetro_m5_rz_v23`
2. mettici dentro `incidenzimetro_m5_rz_v23.ino` e i due file `.h`, tutti allo stesso livello
3. apri il `.ino` dall'IDE: i due `.h` compaiono come schede aggiuntive in alto, segno che sono stati riconosciuti

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

## File inclusi

| File | Descrizione |
|---|---|
| `incidenzimetro_m5_rz_v23.ino` | Programma principale (**versione corrente**) |
| `incidenzimetro_m5_rz_v20.ino` | Versione precedente (due strumenti: Incidenzimetro e Inclinometro) |
| `incidenzimetro_m5_rz_v13.ino` | Versione storica (solo incidenzimetro) |
| `README_Inc.md` | Questo documento |

Da aggiungere al repository: `dati_test_nmea.h` e `dati_test_nmea_sintetici.h` (necessari per compilare la v23).

---

## Suggerimenti involucro/slitta

- Stampa la base con la **faccia inferiore sul piano di stampa** per massima planarità
- Aggiungi un'**aletta di registro** sul bordo d'uscita per posizionamento ripetibile
- Lascia apertura per il **connettore USB-C** (ricarica senza smontare) e, se usi il Log volo da SD, per l'**Hat2-Bus**
- Non aprire l'involucro della M5StickS3: l'antenna WiFi è nel guscio

---

## Cronologia versioni principali

| Versione | Novità |
|---|---|
| **v23** | Versione corrente: tre strumenti (Confronto Escursioni, Goniometro, Log volo) |
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
