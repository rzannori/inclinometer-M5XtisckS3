/*
  ============================================================================
  RZ TOOLS  -  M5StickS3 / M5StickC Plus2  (libreria M5Unified)
  v19: tara bloccante (BLU avanza solo dopo DX, altrimenti bip+scritta grigia);
       BLU torna al menu da qualsiasi schermata finale (matrice, INCL, INTRO, HELP).
  ============================================================================
  Applicazione con due strumenti selezionabili dal menu iniziale:

  1) INCIDENZIMETRO
     Misura l'escursione angolare (gradi) e lineare (mm) delle superfici mobili
     di aeromodelli RC. Sequenza guidata: imposta R (distanza cerniera/bordo
     uscita), tara sul neutro, poi 4 letture (SX alto, SX basso, DX alto,
     DX basso). Schermata finale con matrice 2x2 e differenze di simmetria,
     visualizzabili sia in gradi sia in mm. Verdetto sonoro di simmetria.

  2) INCLINOMETRO
     Misura l'inclinazione in tempo reale con angolo (gradi) e spostamento (mm).
     Sequenza guidata: imposta R (distanza punto di misura), tara sul neutro,
     poi lettura live continua. Feedback sonoro crescente: più inclini, più
     il tono è acuto e i tick più fitti.

  HARDWARE
     M5StickS3 (IMU BMI270) oppure M5StickC Plus2 (IMU MPU6886).
     Tasti usabili: DX (frontale) = M5.BtnB   e   BLU (sopra) = M5.BtnA.
     (il tasto di accensione non è usabile: spegne la scheda)

  FLUSSO SCHERMATE
     LOGO -> MENU -> Incidenzimetro: INTRO->COMANDI->SETUP_R->TARA->LETTURE->MATRICE
                  -> Inclinometro:  INCL_R->INCL_TARA->INCL (live)

  COMANDI (in lettura)
     click DX     = registra il valore corrente (incidenzimetro) / tara (inclinometro)
     tieni DX     = imposta R
     click BLU    = avanti nella sequenza / torna al menu (inclinometro)
     2 click DX   = azzera e torna al menu (nella schermata risultati)
     DX/BLU +1/-1, 2 click +10/-10 nella pagina IMPOSTA R
*/

#include <M5Unified.h>
#include <Preferences.h>
#include <math.h>

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
              INCL_R, INCL_TARA, INCL };  // INCL_R e INCL_TARA = percorso guidato inclinometro
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
int   menuSel  = 0;                     // voce selezionata nel menu (0=Incidenzimetro, 1=Inclinometro)
uint32_t lastTick = 0;                  // tempo dell'ultimo "tick" sonoro dell'inclinometro
float sxU = 0, sxD = 0, dxU = 0, dxD = 0;  // SX alto, SX basso, DX alto, DX basso (gradi)
uint32_t lastDraw = 0;

bool     soundOn      = true;
uint32_t lastBattCheck= 0;

bool     pendA = false; uint32_t tA = 0;   // per doppio click DX
bool     pendB = false; uint32_t tB = 0;   // per doppio click BLU

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
  canvas.drawString("SCEGLI FUNZIONE", 120, 6);

  const char* items[2] = { "Incidenzimetro", "Inclinometro" };
  for (int i = 0; i < 2; i++) {
    int y = 36 + i * 38;                       // riquadro della voce
    if (menuSel == i) {                        // voce selezionata: evidenziata
      canvas.fillRoundRect(20, y, 200, 28, 5, COL_BLU);
      canvas.setTextColor(TFT_BLACK, COL_BLU);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setTextDatum(middle_center);
    canvas.drawString(items[i], 120, y + 14);
  }

  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString("DX cambia   BLU conferma", 120, 133);
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
  canvas.drawString("INCLINOMETRO - R (mm)", 120, 3);
  canvas.drawString("distanza punto di misura", 120, 22);
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_NAVY);
  canvas.setTextDatum(middle_center);
  snprintf(buf, sizeof(buf), "%d", (int)Ri_mm);
  canvas.drawString(buf, 120, 75);
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
  char buf[12];
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("INCLINOMETRO - TARA", 120, 3);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.drawString("Appoggia sul neutro", 120, 22);
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%.1f", liveDegI());  // angolo tarato dell'inclinometro (va a 0 dopo la tara)
  canvas.drawString(buf, 150, 68);
  canvas.drawCircle(163, 50, 3, TFT_GREEN);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(bottom_left);  canvas.drawString("DX = azzera", 6, 133);
  canvas.setTextColor(taredOk ? COL_BLU : TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_right); canvas.drawString(taredOk ? "BLU = avanti" : "prima fai tara!", 236, 133);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCLINOMETRO live: angolo + spostamento in mm
// ============================================================================
void drawIncl() {
  char buf[14];
  float deg = liveDegI();
  float mm  = throwMmI(deg);
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("INCLINOMETRO", 120, 3);

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
//  INTRO ANIMATA
// ============================================================================
void drawIntro() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("INCIDENZIMETRO RZ", 120, 3);

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
  canvas.drawString("IMPOSTA R (mm)", 120, 3);

  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_NAVY);
  canvas.setTextDatum(middle_center);
  snprintf(buf, sizeof(buf), "%d", (int)R_mm);
  canvas.drawString(buf, 120, 52);

  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(top_center);
  canvas.setTextColor(TFT_ORANGE, TFT_NAVY);  canvas.drawString("DX: +1   (2 click = +10)", 120, 82);
  canvas.setTextColor(COL_BLU,   TFT_NAVY);   canvas.drawString("BLU: -1  (2 click = -10)", 120, 100);
  canvas.setTextColor(TFT_YELLOW,TFT_NAVY);   canvas.drawString("tieni un tasto = conferma", 120, 120);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  PAGINA TARA (azzeramento sul neutro)
// ============================================================================
void drawTara() {
  char buf[12];
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("TARA - azzera", 120, 3);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.drawString("Appoggia sul neutro", 120, 22);

  // angolo live grande (deve andare vicino a 0 dopo la tara)
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%.1f", liveDeg);
  canvas.drawString(buf, 150, 70);
  canvas.drawCircle(163, 52, 3, TFT_GREEN);

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
      if (aClick) { menuSel = (menuSel + 1) % 2; snd(2000, 40); }              // DX = scorri la scelta
      if (bClick) {                                                            // BLU = conferma
        if (menuSel == 0) { screen = INTRO; needStaticDraw = true; }           // Incidenzimetro
        else              { screen = INCL_R; pendA = pendB = false; lastDraw = 0; }  // Inclinometro -> percorso guidato
        snd(2400, 60); lastDraw = 0;
      }
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

  switch (screen) {
    case LOGO:     if (millis() - lastDraw >= 40)      { drawLogo();    lastDraw = millis(); } break;
    case MENU:     if (millis() - lastDraw >= DRAW_MS) { drawMenu();    lastDraw = millis(); } break;
    case INCL_R:    if (millis() - lastDraw >= DRAW_MS) { drawInclR();    lastDraw = millis(); } break;
    case INCL_TARA: if (millis() - lastDraw >= DRAW_MS) { drawInclTara(); lastDraw = millis(); } break;
    case INCL:      inclSound(); if (millis() - lastDraw >= DRAW_MS) { drawIncl(); lastDraw = millis(); } break;
    case INTRO:    if (millis() - lastDraw >= 40)      { drawIntro();   lastDraw = millis(); } break;
    case HELP:     if (needStaticDraw)                 { drawHelp();    needStaticDraw = false; } break;
    case SETUP_R:  if (millis() - lastDraw >= DRAW_MS) { drawSetup();   lastDraw = millis(); } break;
    case TARA_SCR: if (millis() - lastDraw >= DRAW_MS) { drawTara();    lastDraw = millis(); } break;
    case MEASURE:  if (millis() - lastDraw >= DRAW_MS) { drawMeasure(); lastDraw = millis(); } break;
  }

  batteryWatch();
  delay(2);
}
