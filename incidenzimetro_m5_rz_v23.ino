/*
  ============================================================================
  RZ TOOLS  -  M5StickS3 / M5StickC Plus2  (libreria M5Unified)
  v22: rinominati gli strumenti (Confronto Escursioni, Goniometro); aggiunta la
       pagina "funzione tasti" anche al Goniometro; testi delle pagine R/tara
       aggiornati; nel Log Volo la scritta in alto a destra ora mostra il
       tempo di riproduzione trascorso invece della sorgente dati.
  ============================================================================
  Applicazione con tre strumenti selezionabili dal menu iniziale:

  1) CONFRONTO ESCURSIONI
     Misura l'escursione angolare (gradi) e lineare (mm) delle superfici mobili
     di aeromodelli RC e confronta SX con DX. Sequenza guidata: imposta la
     distanza tra bordo di uscita e cerniera, tara sul neutro, poi 4 letture
     (SX alto, SX basso, DX alto, DX basso). Schermata finale con matrice 2x2
     e differenze di simmetria, visualizzabili sia in gradi sia in mm.
     Verdetto sonoro di simmetria.

  2) GONIOMETRO
     Misura l'inclinazione in tempo reale con angolo (gradi) e spostamento (mm).
     Sequenza guidata: pagina funzione tasti, imposta la distanza tra bordo
     di uscita e cerniera, tara sul neutro, poi lettura live continua.
     Feedback sonoro crescente: più inclini, più il tono è acuto e i tick
     più fitti.

  3) LOG VOLO (GPS/vario)
     Legge un file di log NMEA esteso (frasi $SMGPS / $SMGPS2, tipiche dei
     logger GPS/vario per alianti RC) e mostra un grafico scorrevole nel
     tempo di vario (m/s), quota (m) o velocità (km/h), a scelta. In alto a
     destra mostra il tempo trascorso dall'inizio della riproduzione corrente.
     Tre sorgenti dati selezionabili: dati di TEST sintetici (per provare
     il grafico senza SD collegata), dati di TEST reali (estratto da un log
     vero, per verificare il parsing), oppure la SD card vera una volta
     collegata (vedi SD_LOG_FILENAME più sotto per il nome del file atteso).
     I dati di test sono negli header dati_test_nmea.h e
     dati_test_nmea_sintetici.h: DEVONO stare nella stessa cartella dello
     sketch (vedi note di caricamento più sotto).

  HARDWARE
     M5StickS3 (IMU BMI270) oppure M5StickC Plus2 (IMU MPU6886).
     Tasti usabili: DX (frontale) = M5.BtnB   e   BLU (sopra) = M5.BtnA.
     (il tasto di accensione non è usabile: spegne la scheda)
     Per il Log Volo con SD vera: modulo SD SPI collegato ai pin
     CS=G1, SCK=G6, MOSI=G7, MISO=G8 (Hat2-Bus dello StickS3).

  FLUSSO SCHERMATE
     LOGO -> MENU -> Confronto Escursioni: INTRO->COMANDI->SETUP_R->TARA->LETTURE->MATRICE
                  -> Goniometro:        INCL_HELP->INCL_R->INCL_TARA->INCL (live)
                  -> Log volo:          LOG_SRC (scegli sorgente) -> LOG_HELP -> LOG_VIEW (grafico live)

  COMANDI (in lettura)
     click DX     = registra il valore corrente (confronto escursioni) / tara (goniometro)
                    / cambia variabile mostrata (log volo, in LOG_VIEW)
     tieni DX     = imposta la distanza (R)
     click BLU    = avanti nella sequenza / torna al menu (goniometro, log volo)
     2 click DX   = azzera e torna al menu (nella schermata risultati)
     DX/BLU +1/-1, 2 click +10/-10 nella pagina di impostazione distanza

  CARICAMENTO NELL'ARDUINO IDE (IMPORTANTE)
     Questo sketch usa due file di dati esterni (dati_test_nmea.h e
     dati_test_nmea_sintetici.h). L'Arduino IDE richiede che la cartella
     dello sketch abbia LO STESSO NOME del file .ino principale, e che i
     file .h stiano nella STESSA cartella. Prima di aprirlo:
       1. crea una cartella chiamata esattamente come questo file, senza
          estensione (es. "incidenzimetro_m5_rz_v21")
       2. mettici dentro questo .ino e i due file .h, tutti allo stesso livello
       3. apri il .ino dall'IDE: i due .h compariranno come schede/tab
          aggiuntive in alto, segno che sono stati riconosciuti correttamente
*/

#include <M5Unified.h>
#include <Preferences.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>
#include "dati_test_nmea.h"           // dati REALI (estratto dal tuo log) per test senza SD
#include "dati_test_nmea_sintetici.h" // dati SINTETICI (vario -10/+10, quota 0-400, vel 0-150)

// ---- Pin SPI per il modulo SD (Hat2-Bus dello StickS3) ----
#define SD_CS_PIN   1
#define SD_SCK_PIN  6
#define SD_MOSI_PIN 7
#define SD_MISO_PIN 8
#define SD_LOG_FILENAME "/log.nmea"   // nome del file da leggere sulla SD vera

// ---- Logo RZ (bitmap 1-bit) ----
// RZ logo 232x54, 1-bit MSB-first
#define LOGO_W 232
#define LOGO_H 54
const uint8_t LOGO_BMP[] = {0,0,0,7,255,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,255,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,255,255,255,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,240,0,0,0,127,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,192,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,252,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,252,0,0,1,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,248,0,0,3,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,224,0,0,7,255,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,192,0,0,63,255,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,128,0,1,255,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,254,0,0,15,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,252,0,0,127,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,248,0,7,255,255,240,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,240,0,63,255,255,128,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,192,3,255,255,248,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,128,31,255,254,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,0,127,255,224,0,0,0,7,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,252,0,127,255,192,0,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,248,0,127,255,224,0,0,0,0,7,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,240,0,127,255,252,0,0,0,0,0,63,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,192,0,63,255,255,255,248,0,0,0,3,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,128,0,15,255,255,255,255,255,252,0,0,63,240,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,255,0,0,1,255,255,255,255,255,255,192,0,0,255,192,63,255,128,0,0,0,0,0,0,0,0,24,255,255,252,0,0,0,15,255,255,255,255,255,240,0,0,7,255,1,255,254,0,0,0,0,0,0,0,0,12,0,63,248,0,0,0,0,0,255,255,255,255,240,0,0,0,63,248,28,0,0,0,0,0,0,0,0,0,60,0,0,0,0,0,0,0,0,0,0,1,255,240,0,0,0,0,255,254,0,0,15,192,0,0,0,3,255,160,0,0,0,0,0,0,0,0,0,0,7,255,224,0,0,0,0,7,255,224,0,127,248,0,0,15,255,192,0,0,0,0,0,0,0,0,0,254,0,31,255,192,0,0,0,0,0,63,255,224,255,252,1,255,252,0,0,0,0,0,0,0,0,0,0,0,127,192,255,255,0,0,0,0,0,0,31,127,255,255,255,255,224,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,254,0,0,0,0,0,0,31,1,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,248,0,0,0,0,0,0,31,0,31,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,224,0,0,0,0,0,0,31,0,127,255,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,0,0,0,0,0,0,0,31,131,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,252,0,0,0,0,0,0,0,31,255,240,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,254,0,0,0,0,0,0,0,31,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,128,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,253,255,0,0,0,0,0,0,31,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,224,112,0,0,0,0,0,7,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,0,0,0,0,0,0,3,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,252,0,0,0,0,0,3,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,248,0,0,0,0,3,255,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,128,3,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,255,255,255,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,255,255,255,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,255,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


const float    EMA_K   = 0.90f;
const uint32_t DRAW_MS = 80;
float          R_mm    = 50.0f;         // R per l'incidenzimetro (mm)
float          Ri_mm   = 50.0f;         // R per l'inclinometro (mm)

const float    SYM_TOL_DEG   = 0.5f;
const uint8_t  SPK_VOL       = 128;
const uint32_t BATT_CHECK_MS = 30000;
const int      BATT_LOW_PCT  = 15;

const uint16_t COL_BLU  = 0x54BF;
const uint32_t DBL_MS   = 300;    // finestra doppio click (ms)
const uint32_t HOLD_MS  = 600;    // soglia "tenuto premuto" (ms)

Preferences prefs;
M5Canvas canvas(&M5.Display);

enum Screen { LOGO, MENU, INTRO, HELP, SETUP_R, TARA_SCR, MEASURE,
              INCL_HELP, INCL_R, INCL_TARA, INCL,
              LOG_SRC, LOG_HELP, LOG_VIEW };  // INCL_HELP = funzione tasti del Goniometro
Screen screen         = LOGO;
Screen afterR         = TARA_SCR;   // dove andare dopo aver confermato R (avvio -> TARA, in misura -> MEASURE)
bool   needStaticDraw = true;

enum Phase { PH_SXU, PH_SXD, PH_GOTO_DX, PH_DXU, PH_DXD, PH_DONE };  // SX alto/basso, cambio, DX alto/basso
Phase phase = PH_SXU;

float zeroOffsetRad = 0.0f;             // tara incidenzimetro
float zeroOffsetRadI= 0.0f;             // tara inclinometro (separata)
float smoothRad     = 0.0f;
bool  smoothInit    = false;
float liveDeg       = 0.0f;
float peakSignedDeg = 0.0f;
bool  measDone = false;                 // true quando le 4 misure sono complete
bool  showMm   = false;                 // non usato nel display ma tenuto per resetFlow
bool  taredOk  = false;                 // true dopo che la tara è stata eseguita almeno una volta
int   menuSel  = 0;                     // voce selezionata nel menu (0=Confronto Escursioni, 1=Goniometro)
uint32_t lastTick = 0;                  // tempo dell'ultimo "tick" sonoro dell'inclinometro
float sxU = 0, sxD = 0, dxU = 0, dxD = 0;  // SX alto, SX basso, DX alto, DX basso (gradi)
uint32_t lastDraw = 0;

bool     soundOn      = true;
uint32_t lastBattCheck= 0;

bool     pendA = false; uint32_t tA = 0;   // per doppio click DX
bool     pendB = false; uint32_t tB = 0;   // per doppio click BLU

// ---- LOG VOLO (lettore NMEA + grafico scorrevole) ------------------------
enum LogSource { SRC_TEST_SYNTH, SRC_TEST_REAL, SRC_SD, SRC_COUNT };
enum LogVar    { VAR_VARIO, VAR_ALT, VAR_SPEED, VAR_COUNT };

LogSource logSrc = SRC_TEST_SYNTH;   // di default: dati sintetici (funziona senza SD collegata)
LogVar    logVar = VAR_VARIO;

const int LOG_GRAPH_W = 198;   // punti visibili nel grafico
const int LOG_GRAPH_X = 34;    // spostato a destra per lasciare posto alla scala
const int LOG_GRAPH_Y = 42;
const int LOG_GRAPH_H = 84;
const uint32_t LOG_PLAY_MS = 120;   // intervallo tra un campione e il successivo (velocita' di scorrimento)

struct LogSample { float alt, speed, vario; };
LogSample logBuf[LOG_GRAPH_W];
int      logWriteIdx    = 0;
int      logCount       = 0;
uint32_t lastLogAdvance = 0;

bool     sdOk           = false;   // esito dell'ultimo tentativo di montare la SD
File     logFile;
size_t   testCursor     = 0;       // cursore di lettura nei dati PROGMEM
bool     pendingAltValid= false;
float    pendingAlt     = 0.0f;
uint32_t logSampleIndex = 0;       // numero di campioni processati dall'inizio della riproduzione corrente
// Intervallo reale tra un campione e il successivo, per ciascuna sorgente:
// il dato sintetico è generato a 0.5s/campione; il file reale campiona a 0.1s/campione
// (le due frasi $SMGPS consecutive nel log originale distano 100ms). Per la SD vera si
// assume in via approssimata lo stesso intervallo del test reale (0.1s), salvo diverso
// intervallo di log del tuo dispositivo GPS: se necessario si può estrarre l'intervallo
// vero leggendo il campo orario delle frasi $GNRMC/$GNGNS invece di questa costante.
const float LOG_SAMPLE_DT_S[SRC_COUNT] = { 0.5f, 0.1f, 0.1f };

// ============================================================================
void snd(uint16_t freq, uint32_t dur) { if (soundOn) M5.Speaker.tone(freq, dur); }

void symmetryVerdict() {
  if (!soundOn) return;
  float dUp   = fabsf(fabsf(sxU) - fabsf(dxU));   // simmetria in ALTO (SX vs DX)
  float dDown = fabsf(fabsf(sxD) - fabsf(dxD));   // simmetria in BASSO
  delay(120);
  if (dUp <= SYM_TOL_DEG && dDown <= SYM_TOL_DEG) { M5.Speaker.tone(2000, 90); delay(110); M5.Speaker.tone(2600, 110); delay(120); }
  else { for (int i = 0; i < 3; i++) { M5.Speaker.tone(600, 120); delay(170); } }
}

// Queste tre devono stare PRIMA di taredRaw() che le usa
float readRawRoll() { float ax, ay, az; M5.Imu.getAccel(&ax, &ay, &az); return atan2f(ay, az); }
float throwMm(float deg)  { return R_mm  * sinf(deg * (float)M_PI / 180.0f); }
float throwMmI(float deg) { return Ri_mm * sinf(deg * (float)M_PI / 180.0f); }

// Media di N campioni freschi per la tara: forza update IMU e legge piu' volte
float taredRaw(int n = 5) {
  float sum = 0;
  for (int i = 0; i < n; i++) {
    M5.Imu.update();                     // forza un nuovo campione dall'IMU
    sum += readRawRoll();                // leggo l'angolo grezzo
    delay(4);                           // piccola pausa tra un campione e l'altro
  }
  return sum / n;                       // media: riduce il rumore da singolo campione
}

void  doTare()  {
  float raw = taredRaw();
  smoothRad     = raw;
  zeroOffsetRad = raw;
  peakSignedDeg = 0.0f;
  taredOk = true;                      // tara eseguita: ora si può avanzare
  snd(2000, 120);
}
void  doTareI() {
  float raw = taredRaw();
  smoothRad      = raw;
  zeroOffsetRadI = raw;
  taredOk = true;                      // tara eseguita: ora si può avanzare
  snd(2000, 120);
}

// liveDegI = angolo live relativo alla tara dell'inclinometro
float liveDegI() { return (smoothRad - zeroOffsetRadI) * 180.0f / (float)M_PI; }

void resetFlow() {
  sxU = sxD = dxU = dxD = 0.0f; measDone = false; showMm = false; taredOk = false;
  phase = PH_SXU; pendA = pendB = false; lastDraw = 0;
  screen = MENU;
  snd(1400, 60);
}

void loadPrefs() {
  prefs.begin("incid", true);
  R_mm  = prefs.getFloat("R",  R_mm);
  Ri_mm = prefs.getFloat("Ri", Ri_mm);
  soundOn = prefs.getBool("snd", true);
  prefs.end();
}
void saveR()  { prefs.begin("incid", false); prefs.putFloat("R",  R_mm);  prefs.end(); }
void saveRi() { prefs.begin("incid", false); prefs.putFloat("Ri", Ri_mm); prefs.end(); }

void batteryWatch() {
  uint32_t now = millis();
  if (now - lastBattCheck < BATT_CHECK_MS) return;
  lastBattCheck = now;
  int pct = M5.Power.getBatteryLevel();
  if (pct >= 0 && pct < BATT_LOW_PCT && soundOn) { M5.Speaker.tone(1000, 80); delay(120); M5.Speaker.tone(1000, 80); }
}

// suono crescente per l'inclinometro: più inclini (in + o in -), più il tono
// è acuto e i "tick" più fitti. Sotto una piccola soglia resta in silenzio.
void inclSound() {
  if (!soundOn) return;
  uint32_t now = millis();
  float a = fabsf(liveDegI());                  // usa l'angolo tarato dell'inclinometro
  uint32_t interval = 250;
  if (a > 1.5f) interval = (uint32_t)constrain(250.0f - a * 3.0f, 70.0f, 250.0f);
  if (now - lastTick >= interval) {
    lastTick = now;
    if (a > 1.5f) {
      uint16_t f = 300 + (uint16_t)(a * 45.0f);
      if (f > 3500) f = 3500;
      M5.Speaker.tone(f, 30);
    }
  }
}

// ============================================================================
//  LOG VOLO: lettura dati (PROGMEM di test o SD vera) + parsing NMEA $SMGPS
// ============================================================================

// Inizializza la SD (da chiamare una volta in setup, non blocca se manca la scheda)
bool sdInit() {
  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  sdOk = SD.begin(SD_CS_PIN, SPI, 25000000);
  return sdOk;
}

// Legge una riga da un buffer PROGMEM (dati di test), avanzando un cursore condiviso.
// Si comporta come file.readStringUntil('\n'): true finche' ci sono dati, false a fine buffer.
bool readTestLineFrom(const char* data, size_t &cursor, String &out) {
  size_t len = strlen_P(data);
  if (cursor >= len) return false;
  out = "";
  while (cursor < len) {
    char c = (char)pgm_read_byte(data + cursor++);
    if (c == '\n') break;
    if (c != '\r') out += c;
  }
  return true;
}

// Apre/riapre la sorgente dati corrente e azzera lo stato di lettura e il grafico.
void openLogSource() {
  testCursor      = 0;
  pendingAltValid = false;
  logWriteIdx     = 0;
  logCount        = 0;
  logSampleIndex  = 0;
  if (logFile) logFile.close();
  if (logSrc == SRC_SD) {
    logFile = SD.open(SD_LOG_FILENAME);
  }
}

// Restituisce la prossima riga grezza dalla sorgente corrente (qualunque essa sia).
bool readNextRawLine(String &out) {
  switch (logSrc) {
    case SRC_TEST_SYNTH: return readTestLineFrom(TEST_NMEA_DATA_SYNTH, testCursor, out);
    case SRC_TEST_REAL:  return readTestLineFrom(TEST_NMEA_DATA,       testCursor, out);
    case SRC_SD:
      if (!logFile || !logFile.available()) return false;
      out = logFile.readStringUntil('\n');
      out.trim();
      return true;
    default: return false;
  }
}

// $SMGPS,<alt> m,... -> estrae l'altitudine (secondo campo)
bool parseAltFromSMGPS(const String &line, float &alt) {
  int c1 = line.indexOf(',');
  if (c1 < 0) return false;
  int c2 = line.indexOf(',', c1 + 1);
  if (c2 < 0) return false;
  alt = line.substring(c1 + 1, c2).toFloat();   // "12.3 m" -> toFloat() legge "12.3"
  return true;
}

// $SMGPS2,X,Y,Z,ENL,us,<speed kmh>,<press>,<press>,<vario m/s>*CS -> velocita' e vario
bool parseSpeedVarioFromSMGPS2(const String &line, float &speed, float &vario) {
  int idx[10]; int n = 0;
  for (int i = 0; i < (int)line.length() && n < 10; i++) if (line[i] == ',') idx[n++] = i;
  if (n < 9) return false;
  speed = line.substring(idx[5] + 1, idx[6]).toFloat();          // campo 6: "xx.x kmh"
  String f9 = line.substring(idx[8] + 1);                        // campo 9: "+x.xx m/s*CS"
  int star = f9.indexOf('*');
  if (star > 0) f9 = f9.substring(0, star);
  vario = f9.toFloat();
  return true;
}

// Legge righe finche' non completa un campione (coppia $SMGPS + $SMGPS2).
// Ignora ogni altra frase NMEA eventualmente presente. false = sorgente esaurita.
bool fetchNextSample(LogSample &s) {
  String line;
  while (readNextRawLine(line)) {
    if (line.length() < 6) continue;
    if (line.startsWith("$SMGPS,")) {
      float a;
      if (parseAltFromSMGPS(line, a)) { pendingAlt = a; pendingAltValid = true; }
    } else if (line.startsWith("$SMGPS2,")) {
      float sp, va;
      if (parseSpeedVarioFromSMGPS2(line, sp, va) && pendingAltValid) {
        s.alt = pendingAlt; s.speed = sp; s.vario = va;
        pendingAltValid = false;
        return true;
      }
    }
  }
  return false;
}

// Avanza la riproduzione di un campione; a fine dati riparte automaticamente dall'inizio.
void logAdvance() {
  LogSample s;
  if (!fetchNextSample(s)) {
    openLogSource();               // fine dati: ricomincia da capo (loop continuo, azzera anche logSampleIndex)
    if (!fetchNextSample(s)) return;   // sorgente vuota/non disponibile: niente da fare
  }
  logBuf[logWriteIdx] = s;
  logWriteIdx = (logWriteIdx + 1) % LOG_GRAPH_W;
  if (logCount < LOG_GRAPH_W) logCount++;
  logSampleIndex++;
}

// ============================================================================
//  LOGO iniziale (RZ) con movimento lento fluttuante
// ============================================================================
void drawLogo() {
  canvas.fillScreen(TFT_BLACK);
  uint32_t t = millis();
  // fluttuazione dolce: due sinusoidi con periodi diversi = movimento "che vola"
  int x = (240 - LOGO_W) / 2 + (int)(4.0f * sinf(2.0f * (float)M_PI * t / 5000.0f));
  int y = 40 + (int)(5.0f * sinf(2.0f * (float)M_PI * t / 3500.0f));
  canvas.drawBitmap(x, y, LOGO_BMP, LOGO_W, LOGO_H, TFT_WHITE);   // logo in bianco su nero

  // invito lampeggiante lento
  if ((t / 700) % 2 == 0) {
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(COL_BLU, TFT_BLACK);
    canvas.setTextDatum(bottom_center);
    canvas.drawString("premi un tasto >", 120, 132);
  }
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  MENU di scelta tra i due programmi
// ============================================================================
void drawMenu() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("SCEGLI FUNZIONE", 120, 2);

  const char* items[3] = { "Confronto Escursioni", "Goniometro", "Log volo (GPS)" };
  for (int i = 0; i < 3; i++) {
    int y = 28 + i * 30;                       // riquadro della voce (piu' distanziato dal titolo)
    if (menuSel == i) {                        // voce selezionata: evidenziata
      canvas.fillRoundRect(20, y, 200, 24, 5, COL_BLU);
      canvas.setTextColor(TFT_BLACK, COL_BLU);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setTextDatum(middle_center);
    canvas.drawString(items[i], 120, y + 12);
  }

  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("DX cambia   BLU conferma", 120, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCL_HELP: funzione tasti del Goniometro (mostrata una volta prima di
//  iniziare, sullo stesso modello della pagina HELP del Confronto Escursioni)
// ============================================================================
void drawInclHelp() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("GONIOMETRO - COMANDI", 120, 3);

  canvas.setTextDatum(top_left);
  canvas.setTextColor(TFT_GREEN,     TFT_BLACK); canvas.drawString("click DX = tara/azzera",   8, 26);
  canvas.setTextColor(TFT_ORANGE,    TFT_BLACK); canvas.drawString("tieni DX = imposta dist.", 8, 46);
  canvas.setTextColor(COL_BLU,       TFT_BLACK); canvas.drawString("click BLU = avanti",       8, 66);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK); canvas.drawString("in lettura: il suono",     8, 90);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK); canvas.drawString("cresce con l'inclinazione",8, 106);

  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("premi per iniziare", 120, 132);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCL_R: impostazione di R per l'inclinometro
// ============================================================================
void drawInclR() {
  char buf[16];
  canvas.fillScreen(TFT_NAVY);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_NAVY);
  canvas.setTextDatum(top_center);
  canvas.drawString("GONIOMETRO - R (mm)", 120, 3);
  canvas.drawString("Distanza bordo uscita", 120, 20);
  canvas.drawString("e cerniera (mm)", 120, 36);
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_NAVY);
  canvas.setTextDatum(middle_center);
  snprintf(buf, sizeof(buf), "%d", (int)Ri_mm);
  canvas.drawString(buf, 120, 84);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_ORANGE, TFT_NAVY);
  canvas.setTextDatum(bottom_left);  canvas.drawString("DX: +1 (2click +10)", 4, 133);
  canvas.setTextColor(COL_BLU, TFT_NAVY);
  canvas.setTextDatum(bottom_right); canvas.drawString("BLU: -1 (tieni=ok)", 236, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCL_TARA: azzeramento per l'inclinometro
// ============================================================================
void drawInclTara() {
  char buf[16];
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("Goniometro", 120, 3);

  if (taredOk) {
    // tara riuscita: messaggio ben visibile, valore diventa piccolo e laterale
    canvas.fillRoundRect(28, 44, 184, 30, 6, TFT_GREEN);
    canvas.setTextColor(TFT_BLACK, TFT_GREEN);
    canvas.setTextDatum(middle_center);
    canvas.drawString("Pronto per la misura", 120, 59);

    canvas.setFont(&fonts::Font0);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(bottom_left);
    snprintf(buf, sizeof(buf), "tara: %+.1f", liveDegI());
    canvas.drawString(buf, 6, 118);
  } else {
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.drawString("Esegui tara", 120, 22);

    canvas.setFont(&fonts::Font7);
    canvas.setTextColor(TFT_GREEN, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%.1f", liveDegI());  // angolo tarato dell'inclinometro (va a 0 dopo la tara)
    canvas.drawString(buf, 150, 68);
    canvas.drawCircle(163, 50, 3, TFT_GREEN);
  }

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(bottom_left);  canvas.drawString("DX = azzera", 6, 133);
  canvas.setTextColor(taredOk ? COL_BLU : TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_right); canvas.drawString(taredOk ? "BLU = avanti" : "prima fai tara!", 236, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  GONIOMETRO live: angolo + spostamento in mm
// ============================================================================
void drawIncl() {
  char buf[14];
  float deg = liveDegI();
  float mm  = throwMmI(deg);
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("Goniometro - Misura", 120, 3);

  // mm GRANDE (valore principale)
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", mm);
  canvas.drawString(buf, 200, 50);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(middle_left);
  canvas.drawString("mm", 204, 50);

  // gradi PICCOLI sotto
  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", deg);
  canvas.drawString(buf, 185, 92);
  canvas.drawCircle(192, 80, 2, TFT_DARKGREY);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  snprintf(buf, sizeof(buf), "R=%dmm", (int)Ri_mm);
  canvas.drawString(buf, 4, 133);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(bottom_center); canvas.drawString("DX=tara", 100, 133);
  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_right);  canvas.drawString("BLU=menu", 236, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  LOG_SRC: scelta della sorgente dati (test sintetico / test reale / SD)
// ============================================================================
void drawLogSrc() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("SORGENTE DATI", 120, 4);

  const char* items[3] = { "Test sintetico", "Test reale (file)", "Scheda SD" };
  for (int i = 0; i < 3; i++) {
    int y = 26 + i * 28;
    if ((int)logSrc == i) {
      canvas.fillRoundRect(20, y, 200, 24, 5, COL_BLU);
      canvas.setTextColor(TFT_BLACK, COL_BLU);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setTextDatum(middle_center);
    canvas.drawString(items[i], 120, y + 12);
  }

  // avviso se e' selezionata la SD ma non risulta montata
  if (logSrc == SRC_SD && !sdOk) {
    canvas.setFont(&fonts::Font0);
    canvas.setTextColor(TFT_RED, TFT_BLACK);
    canvas.setTextDatum(top_center);
    canvas.drawString("SD non rilevata!", 120, 112);
  }

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("DX cambia   BLU conferma", 120, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  LOG_HELP: istruzioni tasti, mostrata una volta prima di entrare nel grafico
//  (cosi' nella schermata del grafico si puo' togliere la legenda e avere
//  piu' spazio per il disegno)
// ============================================================================
void drawLogHelp() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("LOG VOLO - COMANDI", 120, 4);

  canvas.setTextDatum(top_left);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.drawString("DX = cambia variabile", 10, 32);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  canvas.drawString("(vario / quota / velocita')", 10, 50);

  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.drawString("BLU = torna al menu", 10, 76);

  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.drawString("il grafico scorre da solo", 10, 100);

  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("premi un tasto per iniziare", 120, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  LOG_VIEW: grafico scorrevole (vario / quota / velocita'), con scala
// ============================================================================
void drawLogView() {
  static const char*   varNames[VAR_COUNT] = { "VARIO m/s", "QUOTA m", "VELOCITA km/h" };
  static const float   varMin[VAR_COUNT]   = { -10.0f, 0.0f,   0.0f   };
  static const float   varMax[VAR_COUNT]   = {  10.0f, 400.0f, 150.0f };
  static const uint16_t varCol[VAR_COUNT]  = { TFT_GREEN, TFT_ORANGE, TFT_CYAN };

  canvas.fillScreen(TFT_BLACK);

  // riga in alto: tutto su una sola riga (nome variabile a sinistra, sorgente a destra)
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString(varNames[logVar], 4, 2);

  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(top_right);
  float elapsedS = logSampleIndex * LOG_SAMPLE_DT_S[logSrc];   // tempo "del log", non del dispositivo
  char timeBuf[10];
  snprintf(timeBuf, sizeof(timeBuf), "%.1fs", elapsedS);
  canvas.drawString(timeBuf, 236, 2);

  // valore corrente = ultimo campione inserito nel buffer, ben visibile al centro
  float mn = varMin[logVar], mx = varMax[logVar];
  if (logCount > 0) {
    int last = (logWriteIdx - 1 + LOG_GRAPH_W) % LOG_GRAPH_W;
    float v = (logVar == VAR_VARIO) ? logBuf[last].vario
            : (logVar == VAR_ALT)   ? logBuf[last].alt
                                     : logBuf[last].speed;
    char buf[16];
    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(varCol[logVar], TFT_BLACK);
    canvas.setTextDatum(top_center);
    snprintf(buf, sizeof(buf), "%+.1f", v);
    canvas.drawString(buf, 120, 16);
  }

  // scala: valore massimo in alto e minimo in basso, a sinistra del grafico
  canvas.setFont(&fonts::Font0);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(top_right);
  char sbuf[8];
  snprintf(sbuf, sizeof(sbuf), "%.0f", mx);
  canvas.drawString(sbuf, LOG_GRAPH_X - 3, LOG_GRAPH_Y);
  canvas.setTextDatum(bottom_right);
  snprintf(sbuf, sizeof(sbuf), "%.0f", mn);
  canvas.drawString(sbuf, LOG_GRAPH_X - 3, LOG_GRAPH_Y + LOG_GRAPH_H);
  if (mn < 0 && mx > 0) {                       // etichetta dello zero, solo se in scala (es. vario)
    canvas.setTextDatum(middle_right);
    int y0 = LOG_GRAPH_Y + LOG_GRAPH_H - (int)((0 - mn) / (mx - mn) * LOG_GRAPH_H);
    canvas.drawString("0", LOG_GRAPH_X - 3, y0);
  }

  // cornice del grafico
  canvas.drawRect(LOG_GRAPH_X - 1, LOG_GRAPH_Y - 1, LOG_GRAPH_W + 2, LOG_GRAPH_H + 2, TFT_DARKGREY);

  // linea dello zero (utile soprattutto per il vario, che e' bipolare)
  if (mn < 0 && mx > 0) {
    int y0 = LOG_GRAPH_Y + LOG_GRAPH_H - (int)((0 - mn) / (mx - mn) * LOG_GRAPH_H);
    canvas.drawLine(LOG_GRAPH_X, y0, LOG_GRAPH_X + LOG_GRAPH_W, y0, TFT_NAVY);
  }

  // curva scorrevole (buffer circolare -> pixel)
  if (logCount > 1) {
    int n = logCount;
    int start = (logWriteIdx - n + LOG_GRAPH_W) % LOG_GRAPH_W;
    for (int x = 0; x < n - 1; x++) {
      int i1 = (start + x) % LOG_GRAPH_W;
      int i2 = (start + x + 1) % LOG_GRAPH_W;
      float v1 = (logVar == VAR_VARIO) ? logBuf[i1].vario : (logVar == VAR_ALT) ? logBuf[i1].alt : logBuf[i1].speed;
      float v2 = (logVar == VAR_VARIO) ? logBuf[i2].vario : (logVar == VAR_ALT) ? logBuf[i2].alt : logBuf[i2].speed;
      v1 = constrain(v1, mn, mx); v2 = constrain(v2, mn, mx);
      int y1 = LOG_GRAPH_Y + LOG_GRAPH_H - (int)((v1 - mn) / (mx - mn) * LOG_GRAPH_H);
      int y2 = LOG_GRAPH_Y + LOG_GRAPH_H - (int)((v2 - mn) / (mx - mn) * LOG_GRAPH_H);
      canvas.drawLine(LOG_GRAPH_X + x, y1, LOG_GRAPH_X + x + 1, y2, varCol[logVar]);
    }
  }

  // nessuna legenda tasti qui sotto: e' stata mostrata prima, in LOG_HELP
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INTRO ANIMATA
// ============================================================================
void drawIntro() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("CONFRONTO ESCURSIONI", 120, 3);

  const float aMin = 10.0f, aMax = 68.0f;
  const uint32_t period = 3200;
  float phaseAng = 2.0f * (float)M_PI * (millis() % period) / period;
  float thetaDeg = aMin + (aMax - aMin) * (0.5f - 0.5f * cosf(phaseAng));
  float th = thetaDeg * (float)M_PI / 180.0f;

  const int Vx = 30, Vy = 108; const float L = 95.0f;
  int Ex = Vx + (int)(L * cosf(th));
  int Ey = Vy - (int)(L * sinf(th));

  canvas.drawLine(Vx, Vy, 150, Vy, TFT_LIGHTGREY);
  float r = 26.0f; int px = Vx + (int)r, py = Vy;
  for (int i = 1; i <= 10; i++) {
    float a = th * i / 10.0f;
    int nx = Vx + (int)(r * cosf(a)), ny = Vy - (int)(r * sinf(a));
    canvas.drawLine(px, py, nx, ny, TFT_CYAN); px = nx; py = ny;
  }
  canvas.drawWideLine(Vx, Vy, Ex, Ey, 3, TFT_ORANGE);
  canvas.fillCircle(Vx, Vy, 3, TFT_WHITE);

  char buf[8];
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);
  canvas.setTextDatum(middle_left);
  snprintf(buf, sizeof(buf), "%d", (int)(thetaDeg + 0.5f));
  canvas.drawString(buf, 126, 96);
  int w = canvas.textWidth(buf);
  canvas.drawCircle(126 + w + 4, 91, 2, TFT_GREEN);

  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("premi un tasto per iniziare", 120, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  ISTRUZIONI = legenda funzioni tasti
// ============================================================================
void drawHelp() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("FUNZIONI TASTI", 120, 3);

  canvas.setTextDatum(top_left);
  canvas.setTextColor(TFT_GREEN,  TFT_BLACK); canvas.drawString("click DX o BLU = registra", 8, 24);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK); canvas.drawString("tieni DX = imposta R",      8, 44);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK); canvas.drawString("2 click = azzera",          8, 64);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK); canvas.drawString("sequenza: SXalto SXbasso", 8, 88);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK); canvas.drawString("          DXalto DXbasso", 8, 104);

  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("premi per definire R", 120, 132);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  LETTURA GUIDATA
// ============================================================================
void drawMeasure() {
  char buf[28];
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);

  if (phase == PH_GOTO_DX) {
    // pagina di passaggio tra la parte mobile SX e la DX
    canvas.setTextColor(TFT_CYAN, TFT_BLACK);
    canvas.setTextDatum(top_center);
    canvas.drawString("Parte mobile SX OK", 120, 12);

    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(COL_BLU, TFT_BLACK);
    canvas.setTextDatum(middle_center);
    canvas.drawString("Premi BLU", 120, 64);

    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(bottom_center);
    canvas.drawString("prossima parte mobile (DX)", 120, 122);
  }
  else if (phase != PH_DONE) {
    // --- intestazione della fase corrente ---
    const char* titolo; uint16_t col; int passo;
    switch (phase) {
      case PH_SXU: titolo = "SX  ALTO";  col = TFT_CYAN;   passo = 1; break;
      case PH_SXD: titolo = "SX  BASSO"; col = TFT_CYAN;   passo = 2; break;
      case PH_DXU: titolo = "DX  ALTO";  col = TFT_ORANGE; passo = 3; break;
      default:     titolo = "DX  BASSO"; col = TFT_ORANGE; passo = 4; break;
    }
    canvas.setTextColor(col, TFT_BLACK);
    canvas.setTextDatum(top_left);   canvas.drawString(titolo, 4, 3);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(top_right);  snprintf(buf, sizeof(buf), "%d/4", passo); canvas.drawString(buf, 236, 3);

    // mm GRANDE (valore principale)
    float mmLive = throwMm(liveDeg);
    canvas.setFont(&fonts::Font7);
    canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%+.1f", mmLive);
    canvas.drawString(buf, 200, 50);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
    canvas.setTextDatum(middle_left);
    canvas.drawString("mm", 204, 50);

    // gradi PICCOLI sotto
    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%+.1f", liveDeg);
    canvas.drawString(buf, 185, 92);
    canvas.drawCircle(192, 80, 2, TFT_DARKGREY);

    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(bottom_center);
    canvas.drawString("click DX o BLU = registra", 120, 133);
  }
  else {
    // --- MATRICE finale: sempre in mm, con gradi sotto ogni valore ---
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(top_left);   canvas.drawString("mm  (gradi)", 4, 2);
    canvas.setTextDatum(top_right);
    canvas.drawString("ALTO", 145, 2);
    canvas.drawString("BASSO", 234, 2);

    // riga SX
    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(TFT_CYAN, TFT_BLACK);
    canvas.setTextDatum(middle_left);   canvas.drawString("SX", 4, 32);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(sxU)); canvas.drawString(buf, 145, 32);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(sxD)); canvas.drawString(buf, 234, 32);
    // gradi piccoli riga SX
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "(%+.1f)", sxU); canvas.drawString(buf, 145, 48);
    snprintf(buf, sizeof(buf), "(%+.1f)", sxD); canvas.drawString(buf, 234, 48);

    // riga DX
    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
    canvas.setTextDatum(middle_left);   canvas.drawString("DX", 4, 70);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(dxU)); canvas.drawString(buf, 145, 70);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(dxD)); canvas.drawString(buf, 234, 70);
    // gradi piccoli riga DX
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "(%+.1f)", dxU); canvas.drawString(buf, 145, 86);
    snprintf(buf, sizeof(buf), "(%+.1f)", dxD); canvas.drawString(buf, 234, 86);

    // differenze di simmetria in mm
    float dUp = fabsf(fabsf(throwMm(sxU)) - fabsf(throwMm(dxU)));
    float dDn = fabsf(fabsf(throwMm(sxD)) - fabsf(throwMm(dxD)));
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
    canvas.setTextDatum(top_center);
    snprintf(buf, sizeof(buf), "difA %.1fmm  difB %.1fmm", dUp, dDn);
    canvas.drawString(buf, 120, 104);

    canvas.setTextColor(COL_BLU, TFT_BLACK);
    canvas.setTextDatum(bottom_left);
    canvas.drawString("BLU = menu", 4, 133);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(bottom_right);
    canvas.drawString("DX 2click = azzera", 236, 133);
  }
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  IMPOSTA R
// ============================================================================
void drawSetup() {
  char buf[16];
  canvas.fillScreen(TFT_NAVY);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_NAVY);
  canvas.setTextDatum(top_center);
  canvas.drawString("Imposta distanza", 120, 2);
  canvas.drawString("bordo uscita - cerniera", 120, 18);

  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_NAVY);
  canvas.setTextDatum(middle_center);
  snprintf(buf, sizeof(buf), "%d", (int)R_mm);
  canvas.drawString(buf, 120, 62);

  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(top_center);
  canvas.setTextColor(TFT_ORANGE, TFT_NAVY);  canvas.drawString("DX: +1   (2 click = +10)", 120, 90);
  canvas.setTextColor(COL_BLU,   TFT_NAVY);   canvas.drawString("BLU: -1  (2 click = -10)", 120, 106);
  canvas.setTextColor(TFT_YELLOW,TFT_NAVY);   canvas.drawString("tieni un tasto = conferma", 120, 122);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  PAGINA TARA (azzeramento sul neutro)
// ============================================================================
void drawTara() {
  char buf[16];
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("Confronto Escursioni", 120, 3);

  if (taredOk) {
    // tara riuscita: messaggio ben visibile, valore diventa piccolo e laterale
    canvas.fillRoundRect(28, 44, 184, 30, 6, TFT_GREEN);
    canvas.setTextColor(TFT_BLACK, TFT_GREEN);
    canvas.setTextDatum(middle_center);
    canvas.drawString("Pronto per la misura", 120, 59);

    canvas.setFont(&fonts::Font0);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(bottom_left);
    snprintf(buf, sizeof(buf), "tara: %+.1f", liveDeg);
    canvas.drawString(buf, 6, 118);
  } else {
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.drawString("Esegui tara", 120, 22);

    // angolo live grande (deve andare vicino a 0 dopo la tara)
    canvas.setFont(&fonts::Font7);
    canvas.setTextColor(TFT_GREEN, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%.1f", liveDeg);
    canvas.drawString(buf, 150, 70);
    canvas.drawCircle(163, 52, 3, TFT_GREEN);
  }

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  canvas.drawString("click DX = azzera", 6, 133);
  // BLU avanti solo se la tara è stata eseguita: colore diverso per indicarlo
  canvas.setTextColor(taredOk ? COL_BLU : TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_right);
  canvas.drawString(taredOk ? "BLU = avanti" : "prima fai tara!", 236, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  setup()
// ============================================================================
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(160);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  M5.Speaker.setVolume(SPK_VOL);

  if (!M5.Imu.isEnabled()) {
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setTextColor(TFT_WHITE, TFT_RED);
    M5.Display.setTextDatum(middle_center);
    M5.Display.drawString("IMU non trovato", M5.Display.width()/2, M5.Display.height()/2);
    for (int i = 0; i < 4; i++) { M5.Speaker.tone(i % 2 ? 500 : 900, 120); delay(160); }
    while (true) delay(100);
  }

  loadPrefs();
  sdInit();   // tenta di montare la SD; se assente non blocca nulla (si usano i dati di test)

  uint32_t t0 = millis(); float acc = 0; int n = 0;
  while (millis() - t0 < 1000) { if (M5.Imu.update()) { acc += readRawRoll(); n++; } delay(5); }
  if (n > 0) { smoothRad = acc / n; smoothInit = true; zeroOffsetRad = smoothRad; }

  if (soundOn) { M5.Speaker.tone(1500, 80); delay(100); M5.Speaker.tone(2200, 100); }
}

// ============================================================================
//  loop()
// ============================================================================
void loop() {
  M5.update();

  // --- RILEVAMENTO ROBUSTO DEI TASTI ---------------------------------------
  // Genero 4 eventi puliti: click (rilascio breve) e hold (tenuto oltre soglia,
  // una sola volta). Uso pressedFor() + un fermo di stato, invece di wasHold(),
  // perché sul tasto frontale wasHold() non scattava in modo affidabile.
  // NOTA: su questo esemplare i tasti sono invertiti rispetto ai nomi della
  // libreria -> DX (frontale) = M5.BtnB ,  BLU (sopra) = M5.BtnA.
  // aClick/aHold = eventi del tasto DX ; bClick/bHold = eventi del tasto BLU.
  static bool aHeld = false, bHeld = false;   // "questo tasto ha già fatto scattare l'hold?"
  bool aClick = false, aHold = false, bClick = false, bHold = false;

  if (M5.BtnB.pressedFor(HOLD_MS)) { if (!aHeld) { aHold = true; aHeld = true; } }  // DX (=BtnB) tenuto
  if (M5.BtnB.wasReleased())       { if (!aHeld) aClick = true; aHeld = false; }    // DX (=BtnB) click
  if (M5.BtnA.pressedFor(HOLD_MS)) { if (!bHeld) { bHold = true; bHeld = true; } }  // BLU (=BtnA) tenuto
  if (M5.BtnA.wasReleased())       { if (!bHeld) bClick = true; bHeld = false; }    // BLU (=BtnA) click
  // -------------------------------------------------------------------------

  if (M5.Imu.update()) {
    float raw = readRawRoll();
    if (!smoothInit) { smoothRad = raw; smoothInit = true; }
    smoothRad = EMA_K * smoothRad + (1.0f - EMA_K) * raw;
  }
  liveDeg = (smoothRad - zeroOffsetRad) * 180.0f / (float)M_PI;

  switch (screen) {
    case LOGO:
      if (aClick || bClick) { screen = MENU; snd(2200, 60); }  // un tasto qualsiasi -> menu di scelta
      break;

    case MENU:
      if (aClick) { menuSel = (menuSel + 1) % 3; snd(2000, 40); }              // DX = scorri la scelta
      if (bClick) {                                                            // BLU = conferma
        if      (menuSel == 0) { screen = INTRO;  needStaticDraw = true; }               // Confronto Escursioni
        else if (menuSel == 1) { screen = INCL_HELP; needStaticDraw = true; }             // Goniometro
        else                   { screen = LOG_SRC; pendA = pendB = false; lastDraw = 0; } // Log volo
        snd(2400, 60); lastDraw = 0;
      }
      break;

    case LOG_SRC:                                              // scelta sorgente dati per il log
      if (aClick) { logSrc = (LogSource)(((int)logSrc + 1) % SRC_COUNT); snd(2000, 40); }
      if (bClick) {
        openLogSource();
        screen = LOG_HELP; needStaticDraw = true; snd(2400, 60);
      }
      break;

    case LOG_HELP:                                              // istruzioni tasti, una tantum
      if (aClick || bClick) {
        screen = LOG_VIEW; lastLogAdvance = 0; lastDraw = 0; snd(2400, 60);
      }
      break;

    case LOG_VIEW:                                              // grafico scorrevole live
      if (aClick) { logVar = (LogVar)(((int)logVar + 1) % VAR_COUNT); snd(2000, 40); }
      if (bClick) { screen = MENU; lastDraw = 0; snd(2200, 60); }
      break;

    case INCL_HELP:                                            // funzione tasti Goniometro (una tantum)
      if (aClick || bClick) { screen = INCL_R; pendA = pendB = false; lastDraw = 0; snd(2400, 60); }
      break;

    case INCL_R:                                               // impostazione R inclinometro
      if (aClick) { if (pendA && millis()-tA<=DBL_MS){Ri_mm+=10;pendA=false;} else{pendA=true;tA=millis();} }
      if (pendA && millis()-tA>DBL_MS){Ri_mm+=1;pendA=false;}
      if (bClick) { if (pendB && millis()-tB<=DBL_MS){Ri_mm-=10;pendB=false;} else{pendB=true;tB=millis();} }
      if (pendB && millis()-tB>DBL_MS){Ri_mm-=1;pendB=false;}
      if (Ri_mm < 1) Ri_mm = 1;
      if (bHold) { saveRi(); snd(1200,200); screen=INCL_TARA; pendA=pendB=false; lastDraw=0; }  // tieni BLU = conferma R
      break;

    case INCL_TARA:                                            // tara inclinometro
      if (aClick) { taredOk = false; doTareI(); }             // DX = azzera e imposta flag
      if (bClick) {
        if (taredOk) { screen=INCL; pendA=pendB=false; lastDraw=0; snd(2400,60); }
        else { snd(400, 200); }                               // tara non fatta: bip basso di avviso
      }
      break;

    case INCL:                                                 // inclinometro live
      if (aClick) { doTareI(); }                              // DX = ri-tara
      if (bClick) { screen=MENU; lastDraw=0; snd(2200,60); } // BLU = torna al menu
      break;

    case INTRO:
      if (aClick || bClick) { screen = HELP; needStaticDraw = true; snd(2200, 60); }  // qualsiasi tasto = avanti
      break;

    case HELP:
      if (aClick || bClick) { screen = SETUP_R; afterR = TARA_SCR; taredOk = false; pendA = pendB = false; lastDraw = 0; snd(2400, 60); }  // qualsiasi tasto = avanti
      break;

    case TARA_SCR:
      if (aClick) { taredOk = false; doTare(); }              // DX = esegui la tara (azzera e imposta flag)
      if (bClick) {
        if (taredOk) { screen = MEASURE; phase = PH_SXU; pendA = pendB = false; lastDraw = 0; snd(2400, 60); }
        else { snd(400, 200); }                               // tara non fatta: bip basso di avviso
      }
      break;

    case MEASURE:
      if (aHold) { screen = SETUP_R; afterR = MEASURE; pendA = pendB = false; snd(1800, 80); lastDraw = 0; break; }  // tieni DX = imposta R

      if (phase == PH_GOTO_DX) {
        if (bClick) { phase = PH_DXU; pendA = pendB = false; snd(2200, 60); lastDraw = 0; }  // BLU = prossima parte mobile (DX)
      }
      else if (phase != PH_DONE) {
        // click DX oppure BLU: registra il valore CORRENTE e avanza alla fase successiva
        if (aClick || bClick) {
          switch (phase) {
            case PH_SXU: sxU = liveDeg; phase = PH_SXD;     snd(2500, 80); break;
            case PH_SXD: sxD = liveDeg; phase = PH_GOTO_DX; snd(2600, 80); break;
            case PH_DXU: dxU = liveDeg; phase = PH_DXD;     snd(2800, 80); break;
            default:     dxD = liveDeg; phase = PH_DONE; measDone = true; snd(3000, 80); symmetryVerdict(); break;
          }
          pendA = pendB = false; lastDraw = 0;
        }
      }
      else {                                                 // PH_DONE (matrice)
        if (bClick) { resetFlow(); }                         // BLU = torna al menu
        // doppio click DX = azzera
        if (aClick) { if (pendA && millis() - tA <= DBL_MS) { resetFlow(); } else { pendA = true; tA = millis(); } }
        if (pendA && millis() - tA > DBL_MS) pendA = false;
      }
      break;

    case SETUP_R:
      if (aClick) { if (pendA && millis() - tA <= DBL_MS) { R_mm += 10; pendA = false; } else { pendA = true; tA = millis(); } }
      if (pendA && millis() - tA > DBL_MS) { R_mm += 1; pendA = false; }
      if (bClick) { if (pendB && millis() - tB <= DBL_MS) { R_mm -= 10; pendB = false; } else { pendB = true; tB = millis(); } }
      if (pendB && millis() - tB > DBL_MS) { R_mm -= 1; pendB = false; }
      if (R_mm < 1) R_mm = 1;
      if (aHold || bHold) { pendA = pendB = false; saveR(); snd(1200, 200); screen = afterR; lastDraw = 0; }  // tieni = conferma R e prosegui
      break;
  }

  // avanzamento della riproduzione del log (indipendente dal refresh del disegno)
  if (screen == LOG_VIEW && millis() - lastLogAdvance >= LOG_PLAY_MS) {
    logAdvance();
    lastLogAdvance = millis();
  }

  switch (screen) {
    case LOGO:     if (millis() - lastDraw >= 40)      { drawLogo();    lastDraw = millis(); } break;
    case MENU:     if (millis() - lastDraw >= DRAW_MS) { drawMenu();    lastDraw = millis(); } break;
    case INCL_HELP: if (needStaticDraw)                 { drawInclHelp(); needStaticDraw = false; } break;
    case INCL_R:    if (millis() - lastDraw >= DRAW_MS) { drawInclR();    lastDraw = millis(); } break;
    case INCL_TARA: if (millis() - lastDraw >= DRAW_MS) { drawInclTara(); lastDraw = millis(); } break;
    case INCL:      inclSound(); if (millis() - lastDraw >= DRAW_MS) { drawIncl(); lastDraw = millis(); } break;
    case LOG_SRC:  if (millis() - lastDraw >= DRAW_MS) { drawLogSrc();  lastDraw = millis(); } break;
    case LOG_HELP: if (needStaticDraw)                 { drawLogHelp(); needStaticDraw = false; } break;
    case LOG_VIEW: if (millis() - lastDraw >= DRAW_MS) { drawLogView(); lastDraw = millis(); } break;
    case INTRO:    if (millis() - lastDraw >= 40)      { drawIntro();   lastDraw = millis(); } break;
    case HELP:     if (needStaticDraw)                 { drawHelp();    needStaticDraw = false; } break;
    case SETUP_R:  if (millis() - lastDraw >= DRAW_MS) { drawSetup();   lastDraw = millis(); } break;
    case TARA_SCR: if (millis() - lastDraw >= DRAW_MS) { drawTara();    lastDraw = millis(); } break;
    case MEASURE:  if (millis() - lastDraw >= DRAW_MS) { drawMeasure(); lastDraw = millis(); } break;
  }

  batteryWatch();
  delay(2);
}
