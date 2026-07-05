/*
  ============================================================================
  INCIDENZIMETRO RZ  -  M5StickS3 / M5StickC Plus2  (libreria M5Unified)
  v13: matrice finale con valori più grandi (Font4) e differenze difA/difB
       sulla stessa riga. Il BLU commuta gradi/mm, doppio click DX = azzera.
  ============================================================================
  Tasti usabili: DX (frontale) = M5.BtnB   e   BLU (sopra) = M5.BtnA.

  PERCORSO ALL'AVVIO
    INTRO -> COMANDI -> [IMPOSTA R] -> [TARA] -> LETTURE (SX / DX)
    - IMPOSTA R: DX +1 (2click +10), BLU -1 (2click -10), tieni un tasto = conferma
    - TARA: appoggia sul neutro, click DX = azzera, click BLU = vai alle letture
    - In LETTURA: tieni DX = torna a IMPOSTA R (poi rientra in misura);
                  tieni BLU = ri-tara al volo

  FUNZIONI TASTI
    click DX  = registra il valore corrente (fase SX o DX)
    tieni DX  = imposta R
    click BLU = avanti (passa alla fase successiva)
    tieni BLU = tara (solo in fase SX)
    2 click (DX o BLU) nella schermata risultati = azzera
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
float          R_mm    = 50.0f;

const float    SYM_TOL_DEG   = 0.5f;
const uint8_t  SPK_VOL       = 128;
const uint32_t BATT_CHECK_MS = 30000;
const int      BATT_LOW_PCT  = 15;

const uint16_t COL_BLU  = 0x54BF;
const uint32_t DBL_MS   = 300;    // finestra doppio click (ms)
const uint32_t HOLD_MS  = 600;    // soglia "tenuto premuto" (ms)

Preferences prefs;
M5Canvas canvas(&M5.Display);

enum Screen { LOGO, INTRO, HELP, SETUP_R, TARA_SCR, MEASURE };
Screen screen         = LOGO;
Screen afterR         = TARA_SCR;   // dove andare dopo aver confermato R (avvio -> TARA, in misura -> MEASURE)
bool   needStaticDraw = true;

enum Phase { PH_SXU, PH_SXD, PH_GOTO_DX, PH_DXU, PH_DXD, PH_DONE };  // SX alto/basso, cambio, DX alto/basso
Phase phase = PH_SXU;

float zeroOffsetRad = 0.0f;
float smoothRad     = 0.0f;
bool  smoothInit    = false;
float liveDeg       = 0.0f;
float peakSignedDeg = 0.0f;
bool  measDone = false;                 // true quando le 4 misure sono complete
bool  showMm   = false;                 // pagina finale: false=gradi, true=mm
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

float readRawRoll() { float ax, ay, az; M5.Imu.getAccel(&ax, &ay, &az); return atan2f(ay, az); }
float throwMm(float deg) { return R_mm * sinf(deg * (float)M_PI / 180.0f); }
void  doTare() { zeroOffsetRad = smoothRad; peakSignedDeg = 0.0f; snd(2000, 120); }

void resetFlow() {
  sxU = sxD = dxU = dxD = 0.0f; measDone = false; showMm = false;
  phase = PH_SXU; pendA = pendB = false; lastDraw = 0;
  screen = INTRO;               // torna alla pagina iniziale dell'inclinometro (non in lettura)
  snd(1400, 60);
}

void loadPrefs() { prefs.begin("incid", true);  R_mm = prefs.getFloat("R", R_mm); soundOn = prefs.getBool("snd", true); prefs.end(); }
void saveR()     { prefs.begin("incid", false); prefs.putFloat("R", R_mm);   prefs.end(); }

void batteryWatch() {
  uint32_t now = millis();
  if (now - lastBattCheck < BATT_CHECK_MS) return;
  lastBattCheck = now;
  int pct = M5.Power.getBatteryLevel();
  if (pct >= 0 && pct < BATT_LOW_PCT && soundOn) { M5.Speaker.tone(1000, 80); delay(120); M5.Speaker.tone(1000, 80); }
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

    // --- angolo live grande (è il valore che verrà registrato) ---
    canvas.setFont(&fonts::Font7);
    canvas.setTextColor(TFT_GREEN, TFT_BLACK);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%.1f", liveDeg);
    canvas.drawString(buf, 188, 56);
    canvas.drawCircle(202, 36, 3, TFT_GREEN);

    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(bottom_center);
    canvas.drawString("click DX o BLU = registra", 120, 133);
  }
  else {
    // --- MATRICE dei 4 valori (gradi oppure mm secondo showMm) ---
    bool mm = showMm;
    float vSxU = mm ? throwMm(sxU) : sxU;
    float vSxD = mm ? throwMm(sxD) : sxD;
    float vDxU = mm ? throwMm(dxU) : dxU;
    float vDxD = mm ? throwMm(dxD) : dxD;

    // intestazione: unità (a sinistra) + colonne ALTO / BASSO
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(top_left);   canvas.drawString(mm ? "mm" : "gradi", 4, 2);
    canvas.setTextDatum(top_right);
    canvas.drawString("ALTO", 145, 2);
    canvas.drawString("BASSO", 234, 2);

    // righe SX / DX con valori GRANDI (Font4)
    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(TFT_CYAN, TFT_BLACK);
    canvas.setTextDatum(middle_left);   canvas.drawString("SX", 4, 36);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%+.1f", vSxU); canvas.drawString(buf, 145, 36);
    snprintf(buf, sizeof(buf), "%+.1f", vSxD); canvas.drawString(buf, 234, 36);

    canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
    canvas.setTextDatum(middle_left);   canvas.drawString("DX", 4, 66);
    canvas.setTextDatum(middle_right);
    snprintf(buf, sizeof(buf), "%+.1f", vDxU); canvas.drawString(buf, 145, 66);
    snprintf(buf, sizeof(buf), "%+.1f", vDxD); canvas.drawString(buf, 234, 66);

    // differenze di simmetria sulla STESSA riga (Font2, giallo)
    float dUp = fabsf(fabsf(vSxU) - fabsf(vDxU));
    float dDn = fabsf(fabsf(vSxD) - fabsf(vDxD));
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
    canvas.setTextDatum(top_center);
    snprintf(buf, sizeof(buf), "difA %.1f   difB %.1f", dUp, dDn);
    canvas.drawString(buf, 120, 92);

    // suggerimenti in basso
    canvas.setTextColor(COL_BLU, TFT_BLACK);
    canvas.setTextDatum(bottom_left);
    canvas.drawString(mm ? "BLU > gradi" : "BLU > mm", 4, 133);
    canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
    canvas.setTextDatum(bottom_right);
    canvas.drawString("2click DX = azzera", 236, 133);
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
  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(bottom_right);
  canvas.drawString("click BLU = avanti", 236, 133);
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
      if (aClick || bClick) { screen = INTRO; needStaticDraw = true; snd(2200, 60); }  // un tasto qualsiasi -> inclinometro
      break;

    case INTRO:
      if (bClick || aClick) { screen = HELP; needStaticDraw = true; snd(2200, 60); }
      break;

    case HELP:
      if (bClick || aClick) { screen = SETUP_R; afterR = TARA_SCR; pendA = pendB = false; lastDraw = 0; snd(2400, 60); }
      break;

    case TARA_SCR:
      if (aClick) { doTare(); }                              // click DX = esegui la tara (azzera)
      if (bClick) { screen = MEASURE; phase = PH_SXU; pendA = pendB = false; lastDraw = 0; snd(2400, 60); }  // click BLU = vai alle letture
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
        if (bClick) { showMm = !showMm; snd(2200, 60); lastDraw = 0; }   // BLU = cambia unità gradi/mm
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
    case INTRO:    if (millis() - lastDraw >= 40)      { drawIntro();   lastDraw = millis(); } break;
    case HELP:     if (needStaticDraw)                 { drawHelp();    needStaticDraw = false; } break;
    case SETUP_R:  if (millis() - lastDraw >= DRAW_MS) { drawSetup();   lastDraw = millis(); } break;
    case TARA_SCR: if (millis() - lastDraw >= DRAW_MS) { drawTara();    lastDraw = millis(); } break;
    case MEASURE:  if (millis() - lastDraw >= DRAW_MS) { drawMeasure(); lastDraw = millis(); } break;
  }

  batteryWatch();
  delay(2);
}
