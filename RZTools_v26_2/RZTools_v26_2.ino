/*
  ============================================================================
  RZ TOOLS  v26.2  -  M5StickS3 / M5StickC Plus2  (libreria M5Unified)
  ============================================================================
  v26.2: app per telefono in tre lingue (IT / EN / FR, scelta salvata sul
         telefono) e guida all'uso con l'hardware necessario. Lo schermo
         dello stick resta in italiano. Modifiche solo in web_app.h.
  v26.1: app per telefono: slider per R; pagina Setup come RZ Ballast
         (colori di sfondo, principale e barra salvati sul telefono; suono
         dello stick on/off; info stick). Nuovo comando "snd" (suono on/off,
         salvato in NVS) e campi "snd"/"ver" nello stato JSON.
  v26:   APP PER TELEFONO via WiFi.
         - menu: nuova voce "WiFi app" che accende/spegne la rete WiFi
           "RZTools" (password "rztools1"); all'accensione il WiFi e' spento.
         - lo stick serve una pagina web (web_app.h) all'indirizzo
           http://192.168.4.1 : sul telefono si vede la schermata in corso con
           comandi adatti al telefono (R scritta col tastierino, pulsanti
           grandi). Su iPhone: Safari -> Condividi -> Aggiungi a Home.
         - i tasti fisici dello stick continuano a funzionare normalmente.
         - file aggiuntivi nella cartella dello sketch: web_app.h, icon_png.h
  v25.5: risultato finale solo in mm: tolti i gradi sotto i valori SX, DX e
         sotto la differenza (troppi dati per lo schermo).
  v25.4: - confronto DX: il valore DX viene registrato solo quando la
           superficie resta ferma per piu' di 1 s oltre lo zero (fine corsa
           raggiunto); a ogni registrazione bip. A destra si vede il valore
           DX registrato: ROSSO finche' non e' uguale al riferimento SX entro
           +/-0.5 mm (CMP_TOL_MM), VERDE quando ci rientra; sotto, la diff.
         - stessa tolleranza in mm (0.5) per i colori del risultato e per il
           verdetto sonoro (prima era 0.5 gradi).
  v25.3: (la v25.2 e' stata scartata)
         - SX = riferimento: registrazione come v25.1 (chiusura automatica);
           dalla schermata "sposta su DX", back ripete la registrazione SX
           senza rifare la tara.
         - DX = confronto live: lettura grande a sinistra; a destra i
           riferimenti Max S / min S e, sotto, quanto la DX manca (-) o supera
           (+), verde entro tolleranza, rosso fuori. Nessun azzeramento: per
           ogni lato vale l'ultima escursione (fine corsa raggiunto o tenuto
           fermo 1 s). Un solo OK porta al risultato.
         - risultato: in basso sempre la differenza di zero DX/SX, verde "OK"
           sotto 1 mm (ZERO_FINAL_MM), rossa "FUORI TOLL." da 1 mm in su.
  v25.1: - pagina R: sotto la freccia DX "Avanti (tieni)", accanto alla
           freccia BLU "back" + "tieni" in verticale; comandi +1/-1 a destra
           del numero; tolta la riga "tieni DX=ok tieni BLU=menu".
         - zero DX fuori tolleranza: la differenza viene mostrata in tempo
           reale (rossa fuori tolleranza, verde "OK" dentro) per poter
           regolare la superficie; "Continua" va avanti in ogni momento e
           registra come zero DX il valore di quell'istante.
  v25: menu iniziale RZ Tools con due sottoprogrammi:
       - GONIOMETRO   : identico alla v24.2 (unica differenza: nella pagina R
                        "tieni BLU" torna al menu).
       - INCLINOMETRO : confronto delle escursioni SX/DX, riscritto:
                        tara una sola volta sulla SX, registrazione automatica
                        di Max/min, chiusura dopo 2 s senza variazioni,
                        controllo dello zero sulla DX, riepilogo finale.
       R e' unica per entrambi i programmi (salvata in NVS).

  PRINCIPIO
     Il sensore misura l'ANGOLO rispetto alla gravita': su una superficie
     rigida e' lo stesso in ogni punto. Per i mm serve R = distanza tra la
     CERNIERA e il PUNTO in cui si vuole l'escursione (bordo d'uscita):
         mm = R * sin(angolo)
     Con cerniera verticale (timone) il modello va messo su un fianco.

  FLUSSO
     LOGO -> MENU -> GONIOMETRO   : IMPOSTA R -> MONTA -> TARA -> LIVE
                  -> INCLINOMETRO : IMPOSTA R -> MONTA SX -> TARA -> REG. SX
                                    -> MONTA DX -> VERIFICA ZERO [-> ALLARME]
                                    -> CONFRONTO DX -> RISULTATO

  INCLINOMETRO - registrazione
     Dopo la tara si muove la superficie a fondo corsa in su e in giu'.
     Vengono registrati Max (positivo) e min (negativo) in mm e gradi.
     Il conteggio di chiusura parte quando sia Max sia min superano 1 mm
     (ARM_MM); ogni nuovo Max/min che supera il precedente di almeno 0.5 mm
     (VAR_MM) lo fa ripartire. Dopo 2 s (NOVAR_MS) senza variazioni la
     registrazione si chiude da sola (conto alla rovescia a schermo).
     DX ("Fine") chiude a mano in qualsiasi momento.
     La DX usa la tara della SX; se al neutro lo zero DX differisce di oltre
     0.5 mm (ZERO_TOL_MM) compare un allarme con la differenza.

  TASTI E FRECCE
     Tasti usabili: DX = M5.BtnB  e  BLU = M5.BtnA.
     Freccia DX in alto al centro (punta in alto) con l'etichetta della
     funzione; freccia BLU a destra al centro (punta a destra) con "back"
     (o "ok" nel menu).
     Il tasto di accensione non e' usabile: spegne la scheda.

  CARICAMENTO: la cartella dello sketch deve chiamarsi "RZTools_v26_2" e
  contenere anche web_app.h e icon_png.h (compaiono come schede nell'IDE).
*/

#include <M5Unified.h>
#include <Preferences.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>
#include "web_app.h"     // pagina web dell'app per telefono
#include "icon_png.h"    // icona dell'app (schermata Home dell'iPhone)

// ---- WiFi dell'app ----
const char* WIFI_SSID = "RZTools";
const char* WIFI_PASS = "rztools1";     // almeno 8 caratteri
const char* FW_VER    = "26.2";

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

// ---- Parametri della tara / attesa di stabilita' ----
const float    STAB_TOL_DEG   = 0.15f;  // oscillazione massima per considerarlo "fermo" (gradi)
const uint32_t STAB_MS        = 1000;   // per quanto tempo deve restare fermo (ms)
const uint32_t TARA_TIMEOUT_MS= 8000;   // se non si stabilizza: bip basso e riprova
const int      TARA_SAMPLES   = 20;     // campioni mediati

// ---- Parametri dell'inclinometro ----
const float    ARM_MM       = 1.0f;     // Max e min devono superare questo valore prima del conteggio
const float    VAR_MM       = 0.5f;     // variazione minima di Max/min che conta come "variazione"
const uint32_t NOVAR_MS     = 2000;     // tempo senza variazioni per chiudere la registrazione
const float    ZERO_TOL_MM  = 0.5f;     // differenza di zero DX/SX oltre cui compare l'allarme
const float    ZERO_FINAL_MM= 1.0f;     // differenza di zero DX/SX per "FUORI TOLL." nel risultato
const float    HALF_MM      = 1.0f;     // oltre +/- questo valore la DX e' "in escursione" (alto/basso)
const float    HOLD_TOL_MM  = 0.2f;     // oscillazione max per considerare tenuto fermo un fine corsa
const uint32_t HOLD_CMP_MS  = 1000;     // tempo di fermo oltre lo zero per registrare il valore DX
const float    CMP_TOL_MM   = 0.5f;     // tolleranza DX/SX (mm): colori del confronto, del risultato e verdetto

float Ri_mm = 50.0f;                    // R unica (mm), salvata in NVS

Preferences prefs;
M5Canvas canvas(&M5.Display);

enum Screen { LOGO, MENU, SETUP_R, STAB_RUN,
              G_MOUNT, G_LIVE,
              I_MOUNT_SX, I_REC, I_MOUNT_DX, I_ZERO_WARN, I_CMP, I_RESULT };
enum Prog     { P_GONIO, P_INCL };
enum StabMode { ST_TARE, ST_CHECK };    // tara (azzera) oppure verifica dello zero DX

Screen   screen   = LOGO;
Prog     prog     = P_GONIO;
StabMode stabMode = ST_TARE;
int      menuSel  = 0;                  // 0 = Goniometro, 1 = Inclinometro, 2 = WiFi app

// ---- WiFi / app ----
WebServer server(80);
bool     wifiOn = false;
bool     vA = false, vAH = false, vB = false, vBH = false;   // "tasti virtuali" premuti dal telefono

float    zeroOffsetRad = 0.0f;          // tara
float    smoothRad     = 0.0f;          // angolo grezzo filtrato (EMA)
bool     smoothInit    = false;

uint32_t lastDraw      = 0;
uint32_t lastTick      = 0;
bool     soundOn       = true;
uint32_t lastBattCheck = 0;

bool     pendA = false; uint32_t tA = 0;   // doppio click DX
bool     pendB = false; uint32_t tB = 0;   // doppio click BLU

// stato dell'attesa di stabilita'
uint32_t taraStart   = 0;
uint32_t stableStart = 0;
float    stableRef   = 0.0f;

// stato dell'inclinometro
int      recSide    = 0;                // 0 = SX, 1 = DX
float    recMaxDeg  = 0, recMinDeg = 0; // Max/min della registrazione in corso (gradi)
float    markMaxMm  = 0, markMinMm = 0; // valori all'ultima "variazione"
uint32_t lastVarMs  = 0;
bool     recArmed   = false;
float    maxS = 0, minS = 0, maxD = 0, minD = 0;   // risultati (gradi)
float    zeroDxDeg  = 0;
// confronto DX
bool     holdDone   = false;            // fermo corrente gia' registrato (un solo bip per fermo)
bool     hasMaxD    = false, hasMinD = false;
float    holdRefMm  = 0;                // riferimento per riconoscere un fine corsa tenuto fermo
uint32_t holdStart  = 0;                // differenza di zero della DX rispetto alla SX (gradi)

// ============================================================================
void snd(uint16_t freq, uint32_t dur) { if (soundOn) M5.Speaker.tone(freq, dur); }

float readRawRoll() { float ax, ay, az; M5.Imu.getAccel(&ax, &ay, &az); return atan2f(ay, az); }
float rad2deg(float r) { return r * 180.0f / (float)M_PI; }
float liveDeg() { return rad2deg(smoothRad - zeroOffsetRad); }
float throwMm(float deg) { return Ri_mm * sinf(deg * (float)M_PI / 180.0f); }

float taredRaw(int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) { M5.Imu.update(); sum += readRawRoll(); delay(4); }
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

// suono crescente del goniometro
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

void go(Screen s) { screen = s; pendA = pendB = false; lastDraw = 0; }

// ============================================================================
//  APP PER TELEFONO: server web, stato in JSON e comandi
// ============================================================================
const char* screenName(Screen s) {
  switch (s) {
    case LOGO: return "LOGO";             case MENU: return "MENU";
    case SETUP_R: return "SETUP_R";       case STAB_RUN: return "STAB_RUN";
    case G_MOUNT: return "G_MOUNT";       case G_LIVE: return "G_LIVE";
    case I_MOUNT_SX: return "I_MOUNT_SX"; case I_REC: return "I_REC";
    case I_MOUNT_DX: return "I_MOUNT_DX"; case I_ZERO_WARN: return "I_ZERO_WARN";
    case I_CMP: return "I_CMP";           case I_RESULT: return "I_RESULT";
  }
  return "?";
}

void sendState() {
  char js[640];
  float d  = liveDeg();
  float sf = (float)(millis() - stableStart) / (float)STAB_MS; if (sf > 1) sf = 1;
  float left = recArmed ? (NOVAR_MS - (float)(millis() - lastVarMs)) / 1000.0f : 0; if (left < 0) left = 0;
  snprintf(js, sizeof(js),
    "{\"s\":\"%s\",\"p\":%d,\"R\":%.0f,\"mm\":%.2f,\"dg\":%.2f,\"b\":%d,"
    "\"sm\":%d,\"sf\":%.2f,\"rx\":%.2f,\"rn\":%.2f,\"arm\":%d,\"left\":%.1f,"
    "\"ms\":%.2f,\"ns\":%.2f,\"md\":%.2f,\"nd\":%.2f,\"hm\":%d,\"hn\":%d,"
    "\"z\":%.2f,\"tol\":%.2f,\"zt\":%.2f,\"zf\":%.2f,\"snd\":%d,\"ver\":\"%s\"}",
    screenName(screen), (int)prog, Ri_mm, throwMm(d), d, (int)M5.Power.getBatteryLevel(),
    (int)stabMode, sf, throwMm(recMaxDeg), throwMm(recMinDeg), recArmed ? 1 : 0, left,
    throwMm(maxS), throwMm(minS), throwMm(maxD), throwMm(minD), hasMaxD ? 1 : 0, hasMinD ? 1 : 0,
    throwMm(zeroDxDeg), CMP_TOL_MM, ZERO_TOL_MM, ZERO_FINAL_MM, soundOn ? 1 : 0, FW_VER);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", js);
}

// comandi dal telefono: dx / dxh / blu / bluh = tasti virtuali;
// gonio / incl = avvia il programma; setr&v= = imposta R e prosegui; menu = torna al menu;
// snd&v=0/1 = suono dello stick spento/acceso
void handleCmd() {
  String k = server.arg("k");
  if      (k == "dx")   vA  = true;
  else if (k == "dxh")  vAH = true;
  else if (k == "blu")  vB  = true;
  else if (k == "bluh") vBH = true;
  else if (k == "gonio" || k == "incl") {
    prog = (k == "gonio") ? P_GONIO : P_INCL; menuSel = (prog == P_GONIO) ? 0 : 1;
    go(SETUP_R); snd(2400, 60);
  }
  else if (k == "setr" && screen == SETUP_R) {
    float v = server.arg("v").toFloat();
    if (v >= 1 && v <= 2000) { Ri_mm = roundf(v); saveRi(); snd(1200, 200); go(prog == P_GONIO ? G_MOUNT : I_MOUNT_SX); }
  }
  else if (k == "menu") { go(MENU); snd(2200, 60); }
  else if (k == "snd") {                                    // suono on/off (salvato sullo stick)
    soundOn = (server.arg("v") == "1");
    prefs.begin("incid", false); prefs.putBool("snd", soundOn); prefs.end();
    snd(2000, 60);
  }
  sendState();
}

void wifiStart() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  server.on("/", []() { server.send_P(200, "text/html", WEB_APP); });
  server.on("/s", sendState);
  server.on("/c", handleCmd);
  server.on("/icon.png", []() { server.send_P(200, "image/png", (const char*)ICON_PNG, ICON_PNG_LEN); });
  server.onNotFound([]() { server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); });
  server.begin();
  wifiOn = true;
}

void wifiStop() {
  server.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiOn = false;
}

// ============================================================================
//  ELEMENTI GRAFICI COMUNI: frecce dei tasti ed etichette
// ============================================================================
// freccia DX in alto al centro (punta in alto) + etichetta sotto
void drawDxArrow(uint16_t col, const char* label, uint16_t bg) {
  canvas.fillTriangle(111, 8, 129, 8, 120, 0, col);
  if (label) {
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(col, bg);
    canvas.setTextDatum(top_center);
    canvas.drawString(label, 120, 10);
  }
}
// freccia BLU a destra al centro (punta a destra)
void drawBluArrow() { canvas.fillTriangle(231, 58, 231, 76, 239, 67, COL_BLU); }
// etichetta verticale (lettere in colonna) accanto alla freccia BLU
void drawBluVLabel(const char* txt, uint16_t bg) {
  int n = strlen(txt);
  int y = 67 - (n * 13) / 2 - 2;
  char c[2] = { 0, 0 };
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(COL_BLU, bg);
  canvas.setTextDatum(top_center);
  for (int i = 0; i < n; i++) { c[0] = txt[i]; canvas.drawString(c, 223, y + i * 13); }
}
// etichetta verticale a due parti: parola principale (Font2) + parola piccola
// (Font0) sotto, entrambe con le lettere in colonna, accanto alla freccia BLU
void drawBluVLabel2(const char* big, const char* small, uint16_t bg) {
  int nb = strlen(big), ns = strlen(small);
  int h = nb * 13 + 4 + ns * 9;
  int y = 67 - h / 2;
  char c[2] = { 0, 0 };
  canvas.setTextColor(COL_BLU, bg);
  canvas.setTextDatum(top_center);
  canvas.setFont(&fonts::Font2);
  for (int i = 0; i < nb; i++) { c[0] = big[i];   canvas.drawString(c, 223, y + i * 13); }
  y += nb * 13 + 4;
  canvas.setFont(&fonts::Font0);
  for (int i = 0; i < ns; i++) { c[0] = small[i]; canvas.drawString(c, 223, y + i * 9); }
}
// testo con pedice: base in Font2, pedice in Font0 abbassato (es. Max con S)
void drawSub(const char* base, const char* sub, int x, int y, uint16_t col, uint16_t bg) {
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(col, bg);
  canvas.setTextDatum(top_left);
  canvas.drawString(base, x, y);
  int w = canvas.textWidth(base);
  canvas.setFont(&fonts::Font0);
  canvas.drawString(sub, x + w + 1, y + 9);
}
// gradi piccoli (Font0) allineati a destra in (xr, y), con simbolo di grado
void drawDegSmall(float deg, int xr, int y, uint16_t col, uint16_t bg) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%+.1f", deg);
  canvas.setFont(&fonts::Font0);
  canvas.setTextColor(col, bg);
  canvas.setTextDatum(top_right);
  canvas.drawString(buf, xr - 5, y);
  canvas.drawCircle(xr - 2, y + 1, 1, col);
}

// ============================================================================
//  ATTESA DI STABILITA' (tara SX oppure verifica zero DX)
// ============================================================================
void startStab(StabMode m) {
  uint32_t now = millis();
  stabMode    = m;
  taraStart   = now;
  stableStart = now;
  stableRef   = smoothRad;
  go(STAB_RUN);
  snd(1800, 40);
}

// true quando lo stick e' rimasto fermo abbastanza; esegue tara o verifica
bool updateStab() {
  uint32_t now = millis();
  if (fabsf(rad2deg(smoothRad - stableRef)) > STAB_TOL_DEG) { stableRef = smoothRad; stableStart = now; }
  if (now - stableStart >= STAB_MS) {
    float raw = taredRaw(TARA_SAMPLES);
    smoothRad = raw;                              // azzera il ritardo del filtro
    if (stabMode == ST_TARE) zeroOffsetRad = raw; // tara
    else zeroDxDeg = rad2deg(raw - zeroOffsetRad);// verifica: differenza di zero DX
    snd(2000, 90); delay(110); snd(2600, 110);
    return true;
  }
  if (now - taraStart >= TARA_TIMEOUT_MS) {
    snd(400, 200);
    taraStart = stableStart = now; stableRef = smoothRad;
  }
  return false;
}

// ============================================================================
//  INCLINOMETRO: registrazione Max/min
// ============================================================================
void startRec(int side) {
  recSide   = side;
  recMaxDeg = recMinDeg = 0;
  markMaxMm = markMinMm = 0;
  recArmed  = false;
  lastVarMs = millis();
  go(I_REC);
}

// ritorna true quando la registrazione e' chiusa (automaticamente)
bool updateRec() {
  uint32_t now = millis();
  float d = liveDeg();
  if (d > recMaxDeg) recMaxDeg = d;
  if (d < recMinDeg) recMinDeg = d;
  float mxMm = throwMm(recMaxDeg), mnMm = throwMm(recMinDeg);
  if (mxMm - markMaxMm >= VAR_MM || markMinMm - mnMm >= VAR_MM) {   // nuova variazione significativa
    markMaxMm = mxMm; markMinMm = mnMm; lastVarMs = now;
    snd(2400, 15);
  }
  recArmed = (mxMm >= ARM_MM && mnMm <= -ARM_MM);
  if (!recArmed) lastVarMs = now;                 // il conteggio parte solo dopo Max e min
  return recArmed && (now - lastVarMs >= NOVAR_MS);
}

// ---- CONFRONTO DX: per ogni lato vale l'ultima escursione ----------------
void startCmp() {
  hasMaxD = hasMinD = false; maxD = minD = 0; holdDone = false;
  holdRefMm = throwMm(liveDeg()); holdStart = millis();
  go(I_CMP);
}

// registra il valore DX solo quando la superficie resta ferma > 1 s oltre lo zero
void updateCmp() {
  uint32_t now = millis();
  float d = liveDeg(), mm = throwMm(d);
  if (fabsf(mm - holdRefMm) > HOLD_TOL_MM) { holdRefMm = mm; holdStart = now; holdDone = false; }
  if (now - holdStart < HOLD_CMP_MS) return;          // non ancora fermo da 1 s
  if (mm > HALF_MM)       { maxD = d; hasMaxD = true; }
  else if (mm < -HALF_MM) { minD = d; hasMinD = true; }
  else return;                                         // fermo vicino allo zero: niente da registrare
  if (!holdDone) { holdDone = true; snd(2400, 40); }   // bip alla registrazione
}

// true se il valore DX coincide con il riferimento SX entro +/- CMP_TOL_MM
bool cmpOk(float refDeg, float dxDeg) {
  return fabsf(fabsf(throwMm(dxDeg)) - fabsf(throwMm(refDeg))) <= CMP_TOL_MM;
}

void symmetryVerdict() {
  if (!soundOn) return;
  delay(120);
  if (cmpOk(maxS, maxD) && cmpOk(minS, minD)) { M5.Speaker.tone(2000, 90); delay(110); M5.Speaker.tone(2600, 110); delay(120); }
  else { for (int i = 0; i < 3; i++) { M5.Speaker.tone(600, 120); delay(170); } }
}

void endRec() {
  if (recSide == 0) { maxS = recMaxDeg; minS = recMinDeg; snd(2600, 120); go(I_MOUNT_DX); }
  else              { maxD = recMaxDeg; minD = recMinDeg; snd(3000, 80); go(I_RESULT); symmetryVerdict(); }
}

// ============================================================================
//  LOGO
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
//  MENU RZ TOOLS
// ============================================================================
void drawMenu() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString("RZ TOOLS", 4, 2);

  const char* items[3] = { "Goniometro", "Inclinometro", wifiOn ? "WiFi app: ON" : "WiFi app: OFF" };
  for (int i = 0; i < 3; i++) {
    int y = 30 + i * 28;
    if (menuSel == i) {
      canvas.fillRoundRect(20, y, 180, 24, 5, COL_BLU);
      canvas.setTextColor(TFT_BLACK, COL_BLU);
    } else {
      canvas.setTextColor(i == 2 && wifiOn ? TFT_GREEN : TFT_WHITE, TFT_BLACK);
    }
    canvas.setTextDatum(middle_center);
    canvas.drawString(items[i], 110, y + 12);
  }
  if (wifiOn) {                                   // dati per collegare il telefono
    canvas.setFont(&fonts::Font0);
    canvas.setTextColor(TFT_GREEN, TFT_BLACK);
    canvas.setTextDatum(top_left);
    canvas.drawString("rete RZTools  pw rztools1", 4, 116);
    canvas.drawString("apri http://192.168.4.1", 4, 126);
  }
  drawDxArrow(TFT_GREEN, "Cambia", TFT_BLACK);
  drawBluArrow();
  drawBluVLabel("ok", TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  SETUP_R: impostazione di R (comune ai due programmi)
// ============================================================================
void drawSetupR() {
  char buf[16];
  canvas.fillScreen(TFT_NAVY);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_NAVY);
  canvas.setTextDatum(top_center);
  canvas.drawString("Imposta Distanza cerniera", 110, 28);
  canvas.drawString("bordo di uscita (mm)", 110, 44);

  // valore di R (grande, a sinistra)
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_GREEN, TFT_NAVY);
  canvas.setTextDatum(middle_center);
  snprintf(buf, sizeof(buf), "%d", (int)Ri_mm);
  canvas.drawString(buf, 62, 93);

  // comandi +1/-1 a destra del numero
  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(top_left);
  canvas.setTextColor(TFT_ORANGE, TFT_NAVY);
  canvas.drawString("DX: +1",        122, 66);
  canvas.drawString("(2 click +10)", 122, 82);
  canvas.setTextColor(COL_BLU, TFT_NAVY);
  canvas.drawString("BLU: -1",       122, 100);
  canvas.drawString("(2 click -10)", 122, 116);

  drawDxArrow(TFT_ORANGE, "Avanti (tieni)", TFT_NAVY);   // tieni DX = conferma
  drawBluArrow();
  drawBluVLabel2("back", "tieni", TFT_NAVY);              // tieni BLU = menu
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  STAB_RUN: attesa di stabilita' con barra (tara o verifica zero DX)
// ============================================================================
void drawStabRun() {
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString(stabMode == ST_TARE ? "TARA IN CORSO" : "VERIFICA ZERO DX", 120, 10);

  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(middle_center);
  canvas.drawString("tieni fermo...", 120, 48);

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
  drawDxArrow(TFT_DARKGREY, nullptr, TFT_BLACK);
  drawBluArrow();
  drawBluVLabel("back", TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  Schermata generica "monta lo stick" (goniometro, inclinometro SX e DX)
// ============================================================================
void drawMountGeneric(const char* label, const char* l1, const char* l2, const char* l3) {
  char buf[16];
  canvas.fillScreen(TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString(l1, 120, 42);
  canvas.drawString(l2, 120, 60);
  canvas.drawString(l3, 120, 78);

  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  snprintf(buf, sizeof(buf), "R=%dmm", (int)Ri_mm);
  canvas.drawString(buf, 4, 133);
  drawDxArrow(TFT_GREEN, label, TFT_BLACK);
  drawBluArrow();
  drawBluVLabel("back", TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  GONIOMETRO LIVE (come v24.2)
// ============================================================================
void drawGLive() {
  char buf[14];
  float deg = liveDeg();
  float mm  = throwMm(deg);
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("Goniometro - Misura", 120, 10);

  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", mm);
  canvas.drawString(buf, 188, 50);
  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(middle_left);
  canvas.drawString("mm", 192, 50);

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
  drawDxArrow(TFT_GREEN, nullptr, TFT_BLACK);
  drawBluArrow();
  drawBluVLabel("back", TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCLINOMETRO: registrazione (live a sinistra, Max/min a destra)
// ============================================================================
void drawIRec() {
  char buf[16];
  float deg = liveDeg();
  const char* sub = recSide == 0 ? "S" : "D";
  canvas.fillScreen(TFT_BLACK);

  // titolo in alto a sinistra
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString(recSide == 0 ? "Superficie SX" : "Superficie DX", 4, 2);

  // --- zona sinistra: misura continua (mm grande, gradi piccoli) ---
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", throwMm(deg));
  canvas.drawString(buf, 136, 54);
  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(middle_left);
  canvas.drawString("mm", 139, 62);

  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", deg);
  canvas.drawString(buf, 126, 96);
  canvas.drawCircle(132, 85, 2, TFT_DARKGREY);

  // stato della chiusura automatica
  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(bottom_left);
  if (recArmed) {
    float left = (NOVAR_MS - (float)(millis() - lastVarMs)) / 1000.0f;
    if (left < 0) left = 0;
    canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
    snprintf(buf, sizeof(buf), "fine tra %.1f s", left);
    canvas.drawString(buf, 4, 133);
  } else {
    canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    canvas.drawString("muovi su e giu'", 4, 133);
  }

  // --- linea verticale di separazione ---
  canvas.drawFastVLine(160, 0, 135, TFT_DARKGREY);

  // --- zona destra: Max (in alto) e min (in basso) ---
  drawSub("Max", sub, 166, 2, TFT_GREEN, TFT_BLACK);
  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);
  canvas.setTextDatum(top_right);
  snprintf(buf, sizeof(buf), "%+.1f", throwMm(recMaxDeg));
  canvas.drawString(buf, 228, 19);
  drawDegSmall(recMaxDeg, 228, 46, TFT_DARKGREY, TFT_BLACK);

  drawSub("min", sub, 166, 80, TFT_CYAN, TFT_BLACK);
  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_right);
  snprintf(buf, sizeof(buf), "%+.1f", throwMm(recMinDeg));
  canvas.drawString(buf, 228, 97);
  drawDegSmall(recMinDeg, 228, 124, TFT_DARKGREY, TFT_BLACK);

  // frecce: DX = Fine (in alto, centro della zona sinistra); BLU = back
  drawDxArrow(TFT_GREEN, "Fine", TFT_BLACK);
  drawBluArrow();
  canvas.setFont(&fonts::Font0);
  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  canvas.drawString("back", 228, 67);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCLINOMETRO: confronto DX (live a sinistra, riferimento SX e scarto a destra)
// ============================================================================
// blocco a destra: "Max S  +15.2" (riferimento), sotto il valore DX registrato
// (rosso finche' non coincide con il riferimento entro +/-0.5 mm, poi verde) e la diff
void drawCmpBlock(bool isMax, int y) {
  char buf[16];
  float ref = isMax ? maxS : minS;
  bool  has = isMax ? hasMaxD : hasMinD;
  float dx  = isMax ? maxD : minD;
  uint16_t lc = isMax ? TFT_GREEN : TFT_CYAN;

  drawSub(isMax ? "Max" : "min", "S", 164, y, lc, TFT_BLACK);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_right);
  snprintf(buf, sizeof(buf), "%+.1f", throwMm(ref));
  canvas.drawString(buf, 228, y);

  canvas.setFont(&fonts::Font0);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString("DX", 164, y + 26);

  canvas.setFont(&fonts::Font4);
  canvas.setTextDatum(top_right);
  if (has) {
    bool ok = cmpOk(ref, dx);
    canvas.setTextColor(ok ? TFT_GREEN : TFT_RED, TFT_BLACK);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(dx));
    canvas.drawString(buf, 228, y + 17);
    canvas.setFont(&fonts::Font0);
    snprintf(buf, sizeof(buf), "diff %+.1f", fabsf(throwMm(dx)) - fabsf(throwMm(ref)));
    canvas.drawString(buf, 228, y + 45);
  } else {
    canvas.setTextColor(TFT_RED, TFT_BLACK);
    canvas.drawString("--", 228, y + 17);
  }
}

void drawICmp() {
  char buf[16];
  float deg = liveDeg();
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_CYAN, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString("Confronto DX", 4, 2);

  // misura live (come nella registrazione SX)
  canvas.setFont(&fonts::Font7);
  canvas.setTextColor(TFT_ORANGE, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", throwMm(deg));
  canvas.drawString(buf, 136, 54);
  canvas.setFont(&fonts::Font2);
  canvas.setTextDatum(middle_left);
  canvas.drawString("mm", 139, 62);

  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  snprintf(buf, sizeof(buf), "%+.1f", deg);
  canvas.drawString(buf, 126, 96);
  canvas.drawCircle(132, 85, 2, TFT_DARKGREY);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  canvas.drawString("fondo corsa, fermo 1s", 4, 133);

  canvas.drawFastVLine(160, 0, 135, TFT_DARKGREY);
  drawCmpBlock(true,  2);
  drawCmpBlock(false, 80);

  drawDxArrow(TFT_GREEN, "OK", TFT_BLACK);
  drawBluArrow();
  canvas.setFont(&fonts::Font0);
  canvas.setTextColor(COL_BLU, TFT_BLACK);
  canvas.setTextDatum(middle_right);
  canvas.drawString("back", 228, 67);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCLINOMETRO: allarme zero DX diverso
// ============================================================================
void drawIZeroWarn() {
  char buf[32];
  float zMm = throwMm(zeroDxDeg);
  bool  ok  = fabsf(zMm) <= ZERO_TOL_MM;
  uint16_t col = ok ? TFT_GREEN : TFT_RED;
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(col, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString(ok ? "ZERO DX OK" : "ATTENZIONE", 4, 2);

  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_center);
  canvas.drawString("Differenza zero DX / SX", 110, 30);

  // differenza in tempo reale
  canvas.setFont(&fonts::Font4);
  canvas.setTextColor(col, TFT_BLACK);
  snprintf(buf, sizeof(buf), "%+.1f mm", zMm);
  canvas.drawString(buf, 110, 52);
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  snprintf(buf, sizeof(buf), "(%+.1f gradi)  toll. %.1f mm", zeroDxDeg, ZERO_TOL_MM);
  canvas.drawString(buf, 110, 80);

  canvas.setTextColor(ok ? TFT_GREEN : TFT_LIGHTGREY, TFT_BLACK);
  canvas.setTextDatum(bottom_center);
  canvas.drawString(ok ? "OK: premi Continua" : "regola la superficie", 110, 133);
  drawDxArrow(TFT_GREEN, "Continua", TFT_BLACK);
  drawBluArrow();
  drawBluVLabel("back", TFT_BLACK);
  canvas.pushSprite(0, 0);
}

// ============================================================================
//  INCLINOMETRO: risultato finale
// ============================================================================
void drawIResult() {
  char buf[16];
  const int xL = 4, xS = 100, xD = 150, xDf = 205;   // colonne (allineate a destra)
  canvas.fillScreen(TFT_BLACK);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
  canvas.setTextDatum(top_left);
  canvas.drawString("RISULTATO", xL, 2);

  // intestazione
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_left);  canvas.drawString("mm", xL, 28);
  canvas.setTextDatum(top_right);
  canvas.drawString("SX", xS, 28); canvas.drawString("DX", xD, 28); canvas.drawString("diff", xDf, 28);

  // righe Max e min
  float sx[2] = { maxS, minS }, dx[2] = { maxD, minD };
  const char* lab[2] = { "Max", "min" };
  for (int r = 0; r < 2; r++) {
    int y = 46 + r * 30;
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(top_left);  canvas.drawString(lab[r], xL, y);
    canvas.setTextDatum(top_right);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(sx[r])); canvas.drawString(buf, xS, y);
    snprintf(buf, sizeof(buf), "%+.1f", throwMm(dx[r])); canvas.drawString(buf, xD, y);
    float dMm  = fabsf(fabsf(throwMm(sx[r])) - fabsf(throwMm(dx[r])));
    canvas.setTextColor(cmpOk(sx[r], dx[r]) ? TFT_GREEN : TFT_RED, TFT_BLACK);
    snprintf(buf, sizeof(buf), "%.1f", dMm); canvas.drawString(buf, xDf, y);
  }

  // in basso: sempre la differenza di zero DX/SX
  char zbuf[32];
  float zMm = throwMm(zeroDxDeg);
  bool  zOk = fabsf(zMm) < ZERO_FINAL_MM;
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(zOk ? TFT_GREEN : TFT_RED, TFT_BLACK);
  canvas.setTextDatum(bottom_left);
  snprintf(zbuf, sizeof(zbuf), zOk ? "zero DX %+.1f mm  OK" : "zero %+.1f mm FUORI TOLL.", zMm);
  canvas.drawString(zbuf, xL, 133);

  drawDxArrow(TFT_GREEN, "Nuova", TFT_BLACK);
  drawBluArrow();
  drawBluVLabel("menu", TFT_BLACK);
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

  // --- RILEVAMENTO TASTI: DX (frontale) = M5.BtnB, BLU (sopra) = M5.BtnA ---
  static bool aHeld = false, bHeld = false;
  bool aClick = false, aHold = false, bClick = false, bHold = false;
  if (M5.BtnB.pressedFor(HOLD_MS)) { if (!aHeld) { aHold = true; aHeld = true; } }
  if (M5.BtnB.wasReleased())       { if (!aHeld) aClick = true; aHeld = false; }
  if (M5.BtnA.pressedFor(HOLD_MS)) { if (!bHeld) { bHold = true; bHeld = true; } }
  if (M5.BtnA.wasReleased())       { if (!bHeld) bClick = true; bHeld = false; }

  // --- comandi dal telefono (WiFi app): richieste servite qui, tasti virtuali sotto ---
  if (wifiOn) server.handleClient();
  if (vA)  { aClick = true; vA  = false; }
  if (vAH) { aHold  = true; vAH = false; }
  if (vB)  { bClick = true; vB  = false; }
  if (vBH) { bHold  = true; vBH = false; }

  if (M5.Imu.update()) {
    float raw = readRawRoll();
    if (!smoothInit) { smoothRad = raw; smoothInit = true; }
    smoothRad = EMA_K * smoothRad + (1.0f - EMA_K) * raw;
  }

  switch (screen) {
    case LOGO:
      if (aClick || bClick) { go(MENU); snd(2200, 60); }
      break;

    case MENU:
      if (aClick) { menuSel = (menuSel + 1) % 3; snd(2000, 40); }
      if (bClick) {
        if (menuSel == 2) {                                            // WiFi app: accendi / spegni
          if (wifiOn) { wifiStop(); snd(1200, 120); } else { wifiStart(); snd(2600, 120); }
          lastDraw = 0;
        } else { prog = (menuSel == 0) ? P_GONIO : P_INCL; go(SETUP_R); snd(2400, 60); }
      }
      break;

    case SETUP_R:
      if (aClick) { if (pendA && millis() - tA <= DBL_MS) { Ri_mm += 10; pendA = false; } else { pendA = true; tA = millis(); } }
      if (pendA && millis() - tA > DBL_MS) { Ri_mm += 1; pendA = false; }
      if (bClick) { if (pendB && millis() - tB <= DBL_MS) { Ri_mm -= 10; pendB = false; } else { pendB = true; tB = millis(); } }
      if (pendB && millis() - tB > DBL_MS) { Ri_mm -= 1; pendB = false; }
      if (Ri_mm < 1) Ri_mm = 1;
      if (aHold) { saveRi(); snd(1200, 200); go(prog == P_GONIO ? G_MOUNT : I_MOUNT_SX); }   // tieni DX = conferma
      else if (bHold) { saveRi(); snd(2200, 60); go(MENU); }                                // tieni BLU = menu
      break;

    case STAB_RUN:
      if (aClick) { startStab(stabMode); break; }
      if (bClick) {                                                    // annulla
        if (stabMode == ST_CHECK)  go(I_MOUNT_DX);
        else if (prog == P_GONIO)  go(SETUP_R);
        else                       go(I_MOUNT_SX);
        snd(2200, 60); break;
      }
      if (updateStab()) {
        if (stabMode == ST_CHECK) {
          if (fabsf(throwMm(zeroDxDeg)) > ZERO_TOL_MM) { go(I_ZERO_WARN); snd(600, 250); }
          else startCmp();
        }
        else if (prog == P_GONIO) { go(G_LIVE); lastTick = millis(); }
        else startRec(0);
      }
      break;

    // ---------------- GONIOMETRO ----------------
    case G_MOUNT:
      if (aClick) startStab(ST_TARE);
      if (bClick) { go(SETUP_R); snd(2200, 60); }
      break;

    case G_LIVE:
      if (aClick) startStab(ST_TARE);
      if (bClick) { go(SETUP_R); snd(2200, 60); }
      break;

    // ---------------- INCLINOMETRO ----------------
    case I_MOUNT_SX:
      if (aClick) startStab(ST_TARE);
      if (bClick) { go(SETUP_R); snd(2200, 60); }
      break;

    case I_REC:
      if (aClick) { endRec(); break; }                                 // DX = Fine (chiusura manuale)
      if (bClick) { go(recSide == 0 ? I_MOUNT_SX : I_MOUNT_DX); snd(2200, 60); break; }
      if (updateRec()) endRec();
      break;

    case I_MOUNT_DX:
      if (aClick) startStab(ST_CHECK);
      if (bClick) { startRec(0); snd(2200, 60); }                       // back = ripeti registrazione SX (stessa tara)
      break;

    case I_CMP:
      if (aClick) { snd(3000, 80); go(I_RESULT); symmetryVerdict(); break; }   // OK = risultato
      if (bClick) { go(I_MOUNT_DX); snd(2200, 60); break; }
      updateCmp();
      break;

    case I_ZERO_WARN:
      zeroDxDeg = liveDeg();                                           // differenza di zero in tempo reale
      if (aClick) { startCmp(); snd(2400, 60); }                       // continua (anche fuori tolleranza)
      if (bClick) { go(I_MOUNT_DX); snd(2200, 60); }                   // riposiziona
      break;

    case I_RESULT:
      if (aClick) { go(I_MOUNT_SX); snd(2400, 60); }                   // nuova misura
      if (bClick) { go(MENU); snd(2200, 60); }
      break;
  }

  if (millis() - lastDraw >= (screen == LOGO ? 40 : DRAW_MS)) {
    switch (screen) {
      case LOGO:        drawLogo(); break;
      case MENU:        drawMenu(); break;
      case SETUP_R:     drawSetupR(); break;
      case STAB_RUN:    drawStabRun(); break;
      case G_MOUNT:     drawMountGeneric("Avvia Tara", "Monta lo stick sulla", "superficie al NEUTRO", "Poi premi Avvia Tara"); break;
      case G_LIVE:      drawGLive(); break;
      case I_MOUNT_SX:  drawMountGeneric("Avvia Tara", "Posiziona lo stick sulla", "superficie SX al NEUTRO", "Poi premi Avvia Tara"); break;
      case I_REC:       drawIRec(); break;
      case I_MOUNT_DX:  drawMountGeneric("Avvia", "Sposta lo stick sulla", "superficie DX al NEUTRO", "Poi premi Avvia"); break;
      case I_ZERO_WARN: drawIZeroWarn(); break;
      case I_CMP:       drawICmp(); break;
      case I_RESULT:    drawIResult(); break;
    }
    lastDraw = millis();
  }
  if (screen == G_LIVE) liveSound();

  batteryWatch();
  delay(2);
}
