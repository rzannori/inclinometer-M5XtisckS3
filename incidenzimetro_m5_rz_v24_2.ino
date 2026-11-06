/*
  ============================================================================
  RZ TOOLS  -  M5StickS3 / M5StickC Plus2  (libreria M5Unified)
  v24.2: - dopo il logo si va direttamente all'impostazione di R (tolta la
           pagina comandi); titolo "Imposta Distanza cerniera / bordo di
           uscita (mm)".
         - schermata di montaggio: sotto la freccia DX la scritta "Avvia Tara"
           (stesso colore della freccia); testo istruzioni tutto uguale per
           dimensione e colore, "Poi premi Avvia Tara".
         - accanto alla freccia BLU la scritta "back" in verticale (blu) nelle
           schermate in cui BLU torna indietro (montaggio, tara, misura).
  v24.1: frecce che indicano la posizione dei tasti fisici:
       - freccia DX  in alto al centro, punta in alto, stesso colore della
         scritta che spiega il tasto DX nella schermata;
       - freccia BLU a destra, centrata in verticale, punta a destra, blu.
       I titoli sono stati abbassati di qualche pixel per far posto alla
       freccia DX. Nessun'altra modifica rispetto alla v24.
  v24: versione ridotta al solo GONIOMETRO (Confronto Escursioni e Log volo
       rimossi temporaneamente, verranno reintegrati in seguito).
       - Tara "ritardata e stabilizzata": premi DX, togli il dito, lo stick
         attende di essere fermo e poi azzera da solo, con bip di conferma.
       - Dopo la tara si passa da soli alla misura live (niente BLU).
       - In misura: DX = rifai la tara, BLU = torna a impostare R.
       - Conferma di R: tieni premuto DX oppure BLU.
       - Non servono piu' i file dati_test_nmea*.h ne' la libreria SD.
  ============================================================================
  GONIOMETRO
     Misura in tempo reale la rotazione di una superficie mobile (alettone,
     elevatore, flap...) rispetto al neutro, in gradi e in mm.

     Il sensore misura l'ANGOLO rispetto alla gravita': su una superficie
     rigida e' lo stesso in ogni punto, quindi la posizione dello stick sulla
     superficie non conta. Per avere i mm serve R = distanza tra la CERNIERA
     e il PUNTO in cui si vuole l'escursione (di solito il bordo d'uscita):
         mm = R * sin(angolo)
     NB: con cerniera verticale (timone) la gravita' non cambia direzione
     rispetto al sensore: il modello va messo su un fianco.

  FLUSSO SCHERMATE
     LOGO -> IMPOSTA R -> MONTA (Avvia Tara = DX) -> TARA IN CORSO -> LIVE
                ^                                                      |
                +---------------------- BLU (back) --------------------+
                              TARA IN CORSO <---- DX (rifai tara) -----+

  COMANDI
     Logo           : un tasto qualsiasi = avanti
     Imposta R      : DX +1  (2 click +10)   BLU -1  (2 click -10)
                      tieni DX o BLU = conferma
     Monta          : click DX = avvia la tara    click BLU = torna a R
     Tara in corso  : click DX = ricomincia       click BLU = annulla (torna a R)
     Live           : click DX = rifai la tara    click BLU = imposta R

  HARDWARE
     M5StickS3 (IMU BMI270) oppure M5StickC Plus2 (IMU MPU6886).
     Tasti usabili: DX (frontale) = M5.BtnB   e   BLU (sopra) = M5.BtnA.
     (il tasto di accensione non e' usabile: spegne la scheda)
*/

#include <M5Unified.h>
#include <Preferences.h>
#include <math.h>

// ---- Logo RZ (bitmap 1-bit) ----
// RZ logo 232x54, 1-bit MSB-first
#define LOGO_W 232
#define LOGO_H 54
const uint8_t LOGO_BMP[] = {0,0,0,7,255,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,255,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,255,255,255,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,240,0,0,0,127,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,192,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,252,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,252,0,0,1,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,248,0,0,3,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,224,0,0,7,255,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,192,0,0,63,255,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,128,0,1,255,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,254,0,0,15,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,252,0,0,127,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,248,0,7,255,255,240,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,240,0,63,255,255,128,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,192,3,255,255,248,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,128,31,255,254,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,0,127,255,224,0,0,0,7,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,252,0,127,255,192,0,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,248,0,127,255,224,0,0,0,0,7,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,240,0,127,255,252,0,0,0,0,0,63,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,192,0,63,255,255,255,248,0,0,0,3,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,128,0,15,255,255,255,255,255,252,0,0,63,240,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,255,0,0,1,255,255,255,255,255,255,192,0,0,255,192,63,255,128,0,0,0,0,0,0,0,0,24,255,255,252,0,0,0,15,255,255,255,255,255,240,0,0,7,255,1,255,254,0,0,0,0,0,0,0,0,12,0,63,248,0,0,0,0,0,255,255,255,255,240,0,0,0,63,248,28,0,0,0,0,0,0,0,0,0,60,0,0,0,0,0,0,0,0,0,0,1,255,240,0,0,0,0,255,254,0,0,15,192,0,0,0,3,255,160,0,0,0,0,0,0,0,0,0,0,7,255,224,0,0,0,0,7,255,224,0,127,248,0,0,15,255,192,0,0,0,0,0,0,0,0,0,254,0,31,255,192,0,0,0,0,0,63,255,224,255,252,1,255,252,0,0,0,0,0,0,0,0,0,0,0,127,192,255,255,0,0,0,0,0,0,31,127,255,255,255,255,224,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,254,0,0,0,0,0,0,31,1,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,248,0,0,0,0,0,0,31,0,31,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,224,0,0,0,0,0,0,31,0,127,255,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,0,0,0,0,0,0,0,31,131,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,252,0,0,0,0,0,0,0,31,255,240,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,255,255,254,0,0,0,0,0,0,0,31,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,128,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,253,255,0,0,0,0,0,0,31,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,224,112,0,0,0,0,0,7,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,0,0,0,0,0,0,3,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,252,0,0,0,0,0,3,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,248,0,0,0,0,3,255,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,128,3,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,255,255,255,255,255,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,255,255,255,255,255,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,255,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// ---- Parametri generali ----
const float    EMA_K         = 0.90f;   // filtro: piu' alto = piu' stabile/lento
const uint32_t DRAW_MS       = 80;      // intervallo di ridisegno (ms)
const uint8_t  SPK_VOL       = 128;
const uint32_t BATT_CHECK_MS = 30000;
const int      BATT_LOW_PCT  = 15;

const uint16_t COL_BLU  = 0x54BF;
const uint32_t DBL_MS   = 300;          // finestra doppio click (ms)
const uint32_t HOLD_MS  = 600;          // soglia "tenuto premuto" (ms)

// ---- Parametri della tara stabilizzata ----
const float    STAB_TOL_DEG   = 0.15f;  // oscillazione massima ammessa per considerarlo "fermo" (gradi)
const uint32_t STAB_MS        = 1000;   // per quanto tempo deve restare fermo prima della tara (ms)
const uint32_t TARA_TIMEOUT_MS= 8000;   // se non si stabilizza entro questo tempo: bip basso e riprova
const int      TARA_SAMPLES   = 20;     // campioni mediati per la tara

float Ri_mm = 50.0f;                    // R del goniometro (mm), salvata in NVS

Preferences prefs;
M5Canvas canvas(&M5.Display);

enum Screen { LOGO, SETUP_R, MOUNT, TARA_RUN, LIVE };
Screen   screen         = LOGO;

float    zeroOffsetRad = 0.0f;          // tara
float    smoothRad     = 0.0f;          // angolo grezzo filtrato (EMA)
bool     smoothInit    = false;
bool     taredOk       = false;         // true dopo almeno una tara riuscita

uint32_t lastDraw      = 0;
uint32_t lastTick      = 0;             // ultimo "tick" sonoro in misura live
bool     soundOn       = true;
uint32_t lastBattCheck = 0;

bool     pendA = false; uint32_t tA = 0;   // doppio click DX
bool     pendB = false; uint32_t tB = 0;   // doppio click BLU

// stato della tara stabilizzata
uint32_t taraStart   = 0;               // inizio del tentativo corrente
uint32_t stableStart = 0;               // da quando la lettura e' ferma
float    stableRef   = 0.0f;            // angolo di riferimento della finestra di stabilita' (rad)

// ============================================================================
// Frecce che indicano dove si trovano i tasti fisici.
//   DX : in alto al centro, punta verso l'alto, colore = scritta del tasto DX
//   BLU: a destra, centrata in verticale, punta a destra, blu
void drawKeyArrows(uint16_t dxCol) {
  canvas.fillTriangle(111, 8, 129, 8, 120, 0, dxCol);       // DX  (alto, centro)
  canvas.fillTriangle(231, 58, 231, 76, 239, 67, COL_BLU);  // BLU (destra, centro)
}

// Scritta "back" in verticale (lettere in colonna) accanto alla freccia BLU
void drawBackLabel(uint16_t bg) {
  const char* t[4] = { "b", "a", "c", "k" };
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(COL_BLU, bg);
  canvas.setTextDatum(top_center);
  for (int i = 0; i < 4; i++) canvas.drawString(t[i], 223, 41 + i * 13);
}

void snd(uint16_t freq, uint32_t dur) { if (soundOn) M5.Speaker.tone(freq, dur); }

float readRawRoll() { float ax, ay, az; M5.Imu.getAccel(&ax, &ay, &az); return atan2f(ay, az); }
float rad2deg(float r) { return r * 180.0f / (float)M_PI; }
float liveDeg() { return rad2deg(smoothRad - zeroOffsetRad); }
float throwMm(float deg) { return Ri_mm * sinf(deg * (float)M_PI / 180.0f); }

// Media di N campioni freschi: forza l'update dell'IMU e legge piu' volte
float taredRaw(int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) {
    M5.Imu.update();
    sum += readRawRoll();
    delay(4);
  }
  return sum / n;
}

void loadPrefs() {
  prefs.begin("incid", true);
  Ri_mm   = prefs.getFloat("Ri", Ri_mm);
  soundOn = prefs.getBool("snd", true);
  prefs.end();
}
void saveRi() { prefs.begin("incid", false); prefs.putFloat("Ri", Ri_mm); prefs.end(); }

void batteryWatch() {
  uint32_t now = millis();
  if (now - lastBattCheck < BATT_CHECK_MS) return;
  lastBattCheck = now;
  int pct = M5.Power.getBatteryLevel();
  if (pct >= 0 && pct < BATT_LOW_PCT && soundOn) { M5.Speaker.tone(1000, 80); delay(120); M5.Speaker.tone(1000, 80); }
}

// suono crescente: piu' inclini (in + o in -), piu' il tono e' acuto e i
// "tick" piu' fitti. Sotto 1.5 gradi resta in silenzio.
void liveSound() {
  if (!soundOn) return;
  uint32_t now = millis();
  float a = fabsf(liveDeg());
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

// ---- Tara stabilizzata -----------------------------------------------------
void startTara() {
  uint32_t now = millis();
  taraStart   = now;
  stableStart = now;
  stableRef   = smoothRad;
  screen      = TARA_RUN;
  lastDraw    = 0;
  snd(1800, 40);
}

// Chiamata a ogni giro di loop mentre si e' in TARA_RUN.
// Ritorna true quando la tara e' stata eseguita.
bool updateTara() {
  uint32_t now = millis();
  // se si muove oltre la tolleranza, la finestra di stabilita' riparte
  if (fabsf(rad2deg(smoothRad - stableRef)) > STAB_TOL_DEG) {
    stableRef   = smoothRad;
    stableStart = now;
  }
  if (now - stableStart >= STAB_MS) {
    float raw     = taredRaw(TARA_SAMPLES);
    smoothRad     = raw;                 // azzera il ritardo del filtro: effetto istantaneo
    zeroOffsetRad = raw;
    taredOk       = true;
    snd(2000, 90); delay(110); snd(2600, 110);   // due note ascendenti = tara OK
    return true;
  }
  if (now - taraStart >= TARA_TIMEOUT_MS) {      // non si stabilizza: avviso e riprova
    snd(400, 200);
    taraStart   = now;
    stableStart = now;
    stableRef   = smoothRad;
  }
  return false;
}

// ============================================================================
//  LOGO iniziale (RZ) con movimento lento fluttuante
// ============================================================================
void drawLogo() {
  canvas.fillScreen(TFT_BLACK);
  uint32_t t = millis();
  int x = (240 - LOGO_W) / 2 + (int)(4.0f * sinf(2.0f * (float)M_PI * t / 5000.0f));
  int y = 40 + (int)(5.0f * sinf(2.0f * (float)M_PI * t / 3500.0f));
  canvas.drawBitmap(x, y, LOGO_BMP, LOGO_W, LOGO_H, TFT_WHITE);

  if ((t / 700) % 2 == 0) {
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(COL_BLU, TFT_BLACK);
    canvas.setTextDatum(bottom_center);
    canvas.drawString("premi un tasto >", 120, 132);
  }
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  SETUP_R: impostazione di R
// ============================================================================
void drawSetupR() {
  char buf[16];
  canvas.fillScreen(TFT_NAVY);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_NAVY);
  canvas.setTextDatum(top_center);
  canvas.drawString("Imposta Distanza cerniera", 120, 9);
  canvas.drawString("bordo di uscita (mm)", 120, 24);

  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_NAVY);
  canvas.setTextDatum(middle_center);
  snprintf(buf, sizeof(buf), "%d", (int)Ri_mm);
  canvas.drawString(buf, 120, 64);

  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(top_center);
  canvas.setTextColor(TFT_ORANGE, TFT_NAVY);  canvas.drawString("DX: +1   (2 click = +10)", 120, 90);
  canvas.setTextColor(COL_BLU,    TFT_NAVY);  canvas.drawString("BLU: -1  (2 click = -10)", 120, 106);
  canvas.setTextColor(TFT_YELLOW, TFT_NAVY);  canvas.drawString("tieni un tasto = conferma", 120, 122);
  drawKeyArrows(TFT_ORANGE);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  MOUNT: monta lo stick sulla superficie al neutro, poi Avvia Tara (DX)
// ============================================================================
void drawMount() {
  char buf[16];
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);              // stesso colore della freccia DX
  canvas.setTextDatum(top_center);
  canvas.drawString("Avvia Tara", 120, 10);

  canvas.setTextColor(TFT_WHITE, TFT_BLACK);              // istruzioni: stessa dimensione e colore
  canvas.drawString("Monta lo stick sulla", 120, 42);
  canvas.drawString("superficie al NEUTRO", 120, 60);
  canvas.drawString("Poi premi Avvia Tara", 120, 78);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  snprintf(buf, sizeof(buf), "R=%dmm", (int)Ri_mm);
  canvas.drawString(buf, 4, 133);
  drawKeyArrows(TFT_GREEN);
  drawBackLabel(TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  TARA_RUN: attesa della stabilita' con barra di avanzamento
// ============================================================================
void drawTaraRun() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("TARA IN CORSO", 120, 10);

  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(middle_center);
  canvas.drawString("tieni fermo...", 120, 48);

  // barra: quanto manca alla tara (si svuota se lo stick si muove)
  uint32_t el = millis() - stableStart;
  float frac = (float)el / (float)STAB_MS;
  if (frac > 1.0f) frac = 1.0f;
  const int bx = 30, by = 76, bw = 180, bh = 16;
  canvas.drawRect(bx, by, bw, bh, TFT_DARKGREY);
  canvas.fillRect(bx + 2, by + 2, (int)((bw - 4) * frac), bh - 4, TFT_GREEN);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  canvas.drawString("DX = ricomincia", 4, 133);
  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_right);
  canvas.drawString("BLU = annulla", 236, 133);
  drawKeyArrows(TFT_DARKGREY);
  drawBackLabel(TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  LIVE: mm grande + gradi piccoli
// ============================================================================
void drawLive() {
  char buf[14];
  float deg = liveDeg();
  float mm  = throwMm(deg);
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("Goniometro - Misura", 120, 10);

  // mm GRANDE (valore principale)
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", mm);
  canvas.drawString(buf, 188, 50);
  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(middle_left);
  canvas.drawString("mm", 192, 50);

  // gradi PICCOLI sotto
  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", deg);
  canvas.drawString(buf, 175, 92);
  canvas.drawCircle(182, 80, 2, TFT_DARKGREY);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  snprintf(buf, sizeof(buf), "R=%dmm", (int)Ri_mm);
  canvas.drawString(buf, 4, 133);
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);
  canvas.setTextDatum(bottom_center); canvas.drawString("DX=tara", 120, 133);
  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_right);  canvas.drawString("BLU=R", 236, 133);
  drawKeyArrows(TFT_GREEN);
  drawBackLabel(TFT_BLACK);
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

  // --- RILEVAMENTO TASTI ----------------------------------------------------
  // click = rilascio breve; hold = tenuto oltre HOLD_MS (una sola volta).
  // NOTA: su questo esemplare DX (frontale) = M5.BtnB, BLU (sopra) = M5.BtnA.
  // aClick/aHold = tasto DX ; bClick/bHold = tasto BLU.
  static bool aHeld = false, bHeld = false;
  bool aClick = false, aHold = false, bClick = false, bHold = false;

  if (M5.BtnB.pressedFor(HOLD_MS)) { if (!aHeld) { aHold = true; aHeld = true; } }
  if (M5.BtnB.wasReleased())       { if (!aHeld) aClick = true; aHeld = false; }
  if (M5.BtnA.pressedFor(HOLD_MS)) { if (!bHeld) { bHold = true; bHeld = true; } }
  if (M5.BtnA.wasReleased())       { if (!bHeld) bClick = true; bHeld = false; }
  // -------------------------------------------------------------------------

  if (M5.Imu.update()) {
    float raw = readRawRoll();
    if (!smoothInit) { smoothRad = raw; smoothInit = true; }
    smoothRad = EMA_K * smoothRad + (1.0f - EMA_K) * raw;
  }

  switch (screen) {
    case LOGO:
      if (aClick || bClick) { screen = SETUP_R; pendA = pendB = false; lastDraw = 0; snd(2200, 60); }
      break;

    case SETUP_R:
      if (aClick) { if (pendA && millis() - tA <= DBL_MS) { Ri_mm += 10; pendA = false; } else { pendA = true; tA = millis(); } }
      if (pendA && millis() - tA > DBL_MS) { Ri_mm += 1; pendA = false; }
      if (bClick) { if (pendB && millis() - tB <= DBL_MS) { Ri_mm -= 10; pendB = false; } else { pendB = true; tB = millis(); } }
      if (pendB && millis() - tB > DBL_MS) { Ri_mm -= 1; pendB = false; }
      if (Ri_mm < 1) Ri_mm = 1;
      if (aHold || bHold) {                                   // tieni un tasto = conferma R
        pendA = pendB = false; saveRi(); snd(1200, 200);
        screen = MOUNT; lastDraw = 0;
      }
      break;

    case MOUNT:
      if (aClick) startTara();                                // DX = avvia la tara stabilizzata
      if (bClick) { screen = SETUP_R; pendA = pendB = false; lastDraw = 0; snd(2200, 60); }
      break;

    case TARA_RUN:
      if (aClick) { startTara(); break; }                     // DX = ricomincia l'attesa
      if (bClick) { screen = SETUP_R; pendA = pendB = false; lastDraw = 0; snd(2200, 60); break; }
      if (updateTara()) { screen = LIVE; lastDraw = 0; lastTick = millis(); }
      break;

    case LIVE:
      if (aClick) startTara();                                // DX = rifai la tara
      if (bClick) { screen = SETUP_R; pendA = pendB = false; lastDraw = 0; snd(2200, 60); }
      break;
  }

  switch (screen) {
    case LOGO:     if (millis() - lastDraw >= 40)      { drawLogo();    lastDraw = millis(); } break;
    case SETUP_R:  if (millis() - lastDraw >= DRAW_MS) { drawSetupR();  lastDraw = millis(); } break;
    case MOUNT:    if (millis() - lastDraw >= DRAW_MS) { drawMount();   lastDraw = millis(); } break;
    case TARA_RUN: if (millis() - lastDraw >= DRAW_MS) { drawTaraRun(); lastDraw = millis(); } break;
    case LIVE:     liveSound(); if (millis() - lastDraw >= DRAW_MS) { drawLive(); lastDraw = millis(); } break;
  }

  batteryWatch();
  delay(2);
}
