// ============================================================================
//  web_app.h  -  pagina web dell'app RZ Tools per telefono (servita dallo stick)
//  v26.2: lingue IT / EN / FR, guida all'uso (con hardware necessario),
//         Setup con colori, lingua, suono dello stick. Stile RZ Ballast Archive.
//  Lo stick risponde a:  GET /     -> questa pagina
//                        GET /s    -> stato in JSON
//                        GET /c?k= -> comando (risponde con lo stato)
// ============================================================================
#pragma once
#include <Arduino.h>

const char WEB_APP[] PROGMEM = R"RZHTML(<!doctype html>
<html lang="it"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="RZ Tools">
<meta name="theme-color" content="#0c1e30">
<link rel="apple-touch-icon" href="/icon.png">
<title>RZ Tools</title>
<style>
:root{--bg:#e9eff5;--card:#fff;--ink:#0f2233;--muted:#6b7f93;--acc:#19b956;--line:#e3eaf1;--h1:#0c1e30;--h2:#14405f;--ok:#17a34a;--mid:#e09400;--bad:#dc3f3f;--shadow:0 4px 16px rgba(12,30,48,.08)}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;font:16px -apple-system,system-ui,"Segoe UI",sans-serif;background:var(--bg);color:var(--ink);padding-bottom:env(safe-area-inset-bottom);-webkit-user-select:none;user-select:none}
header{background:linear-gradient(135deg,var(--h1),var(--h2));color:#fff;padding:calc(env(safe-area-inset-top) + 10px) 16px 10px;display:flex;align-items:center;gap:10px;position:sticky;top:0;z-index:5}
header .brand{display:flex;align-items:center;gap:8px;background:none;border:0;color:#fff;font:inherit;padding:9px 14px 9px 0;margin:-9px 0;cursor:pointer}
header .brand:active{opacity:.6}
header .t{font-weight:800;letter-spacing:.08em;font-size:15px;text-transform:uppercase}
header .t span{opacity:.75}
#conn{margin-left:auto;display:flex;align-items:center;gap:6px;font-size:13px;font-weight:700;opacity:.9}
#conn i{width:10px;height:10px;border-radius:5px;background:var(--bad);display:inline-block}
#conn.on i{background:#2be06a}
main{padding:14px}
.card{background:var(--card);border-radius:18px;padding:16px;margin-bottom:14px;box-shadow:var(--shadow)}
.mtop{border-left:6px solid var(--acc);padding:12px 16px}
.mlab{font-size:11px;font-weight:800;letter-spacing:.09em;text-transform:uppercase;color:var(--acc)}
.mbig{font-size:24px;font-weight:800;line-height:1.15;margin:3px 0 2px}
h2{margin:0 0 10px;font-size:12px;text-transform:uppercase;letter-spacing:.09em;color:var(--acc);font-weight:800}
.muted{color:var(--muted);font-size:14px;line-height:1.4}
p.txt{margin:0 0 6px;font-size:17px;line-height:1.4}
.cgbox{text-align:center;padding:14px 6px 16px;border-radius:14px;background:var(--bg)}
.cgbox .v{font-size:64px;font-weight:800;line-height:1.05;font-variant-numeric:tabular-nums}
.cgbox .v small{font-size:20px;font-weight:600;color:var(--muted)}
.cgbox .d{font-size:18px;color:var(--muted);font-variant-numeric:tabular-nums}
.v.ok{color:var(--ok)}.v.bad{color:var(--bad)}
.stat{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:12px}
.stat div{background:var(--bg);border-radius:14px;padding:11px 12px}
.stat span{font-size:12px;font-weight:700;color:var(--muted);text-transform:uppercase;letter-spacing:.05em}
.stat b{display:block;font-size:28px;margin-top:2px;font-variant-numeric:tabular-nums}
sub{font-size:.7em}
.cmp{display:grid;grid-template-columns:52px 1fr 1fr;gap:8px;align-items:center;padding:10px 0;border-bottom:1px solid var(--line)}
.cmp:last-child{border:0}
.cmp .k{font-weight:800}
.cmp .c{background:var(--bg);border-radius:12px;padding:8px 10px;text-align:center}
.cmp .c span{display:block;font-size:11px;font-weight:700;color:var(--muted);letter-spacing:.05em}
.cmp .c b{font-size:24px;font-variant-numeric:tabular-nums}
.cmp .c.ok{background:var(--ok);color:#fff}.cmp .c.bad{background:var(--bad);color:#fff}
.cmp .c.ok span,.cmp .c.bad span{color:#fff;opacity:.85}
.cmp .df{grid-column:2/4;text-align:right;font-size:13px;color:var(--muted)}
table{width:100%;border-collapse:collapse;font-size:17px}
th{font-size:11px;text-transform:uppercase;letter-spacing:.05em;color:var(--muted)}
td,th{padding:10px 3px;text-align:center;border-bottom:1px solid var(--line);font-variant-numeric:tabular-nums}
td:first-child{text-align:left;font-weight:800}
.st{display:inline-block;padding:4px 12px;border-radius:14px;font-weight:700;color:#fff}
.st.ok{background:var(--ok)}.st.bad{background:var(--bad)}.st.mid{background:var(--mid)}
.zrow{margin-top:12px;display:flex;justify-content:space-between;align-items:center}
.btns{display:flex;flex-direction:column;gap:10px;margin-top:4px}
button.act{padding:15px;border-radius:12px;border:1.5px solid var(--line);background:var(--card);color:var(--ink);width:100%;font-size:16px;font-weight:600;font-family:inherit}
button.pri{background:var(--acc);color:#fff;border:0;box-shadow:0 4px 12px rgba(0,0,0,.2);font-size:18px;font-weight:700;padding:17px}
button.act:active{transform:scale(.98)}
.tool{display:block;width:100%;text-align:left;border:0;font:inherit;color:inherit;cursor:pointer}
.tool:active{transform:scale(.99)}
.tool .go{float:right;font-size:26px;color:var(--acc);font-weight:800;line-height:1}
.rrow{display:flex;align-items:center;gap:10px}
.rrow button{width:54px;height:54px;font-size:28px;border-radius:27px;border:0;background:linear-gradient(135deg,var(--h1),var(--h2));color:#fff;box-shadow:0 3px 8px rgba(12,30,48,.3);flex:none}
.rrow button:active{transform:scale(.94)}
.rrow input{flex:1;min-width:0;font-size:46px;font-weight:800;text-align:center;border:1.5px solid var(--line);border-radius:14px;padding:6px;background:var(--bg);color:var(--ink);font-family:inherit;-webkit-user-select:text;user-select:text}
.bar{height:16px;border-radius:8px;background:var(--bg);overflow:hidden;margin:14px 0 4px}
.bar i{display:block;height:100%;width:0;background:var(--acc);transition:width .2s}
.hero{background:linear-gradient(135deg,var(--h1),var(--h2));color:#fff;border-radius:18px;padding:26px 18px;margin-bottom:14px;box-shadow:var(--shadow)}
.hero h1{margin:0;font-size:28px;letter-spacing:.06em;font-weight:900;text-transform:uppercase}
.hero h1 span{color:var(--acc)}
.hero p{margin:8px 0 0;font-size:12px;letter-spacing:.14em;text-transform:uppercase;color:#cfe0ef}
#off{position:fixed;inset:0;z-index:20;background:rgba(0,0,0,.55);display:none;align-items:center;justify-content:center;padding:24px}
#off.on{display:flex}
#off .card{max-width:420px}
#off h3{margin:0 0 8px;color:var(--bad)}
input[type=range]{width:100%;margin:16px 0 0;accent-color:var(--acc);height:28px}
.rlim{display:flex;justify-content:space-between;font-size:12px;color:var(--muted)}
h2.hq{display:flex;align-items:center;justify-content:space-between;gap:8px}
.q{width:24px;height:24px;border-radius:12px;border:1.5px solid currentColor;background:none;color:var(--muted);font-size:13px;font-weight:700;line-height:1;padding:0;flex:none;opacity:.85;font-family:inherit}
.sw{display:inline-flex;align-items:center;justify-content:center;width:44px;height:44px;border-radius:22px;border:3px solid transparent;margin:4px 6px 4px 0;vertical-align:middle;padding:0}
.sw.on{border-color:var(--ink)}
.sw.bgsw{width:auto;border-radius:12px;padding:0 14px;font-size:14px;font-weight:600;border:2.5px solid var(--line);font-family:inherit}
.sw.bgsw.on{border-color:var(--acc)}
.set{display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid var(--line);gap:8px}
.set:last-child{border:0}
.tg{position:relative;width:52px;height:32px;border-radius:16px;border:0;background:var(--line);flex:none;padding:0}
.tg::after{content:"";position:absolute;top:3px;left:3px;width:26px;height:26px;border-radius:13px;background:#fff;box-shadow:0 1px 3px rgba(0,0,0,.3);transition:left .2s}
.tg.on{background:var(--acc)}.tg.on::after{left:23px}
.tool .gear{float:right;font-size:22px;line-height:1}
#helpm{position:fixed;inset:0;z-index:30;background:rgba(0,0,0,.55);display:none;align-items:center;justify-content:center;padding:24px}
#helpm.on{display:flex}
#helpm .card{max-width:420px;width:100%;max-height:80vh;overflow:auto}
#helpm h3{margin:0 0 8px;color:var(--acc)}
#helpm p{margin:0 0 14px;line-height:1.45;font-size:15px}

.hero{position:relative}
.langs{display:flex;gap:6px;flex-wrap:wrap}
.langs.l1{position:absolute;top:12px;right:12px}
.langs button{padding:5px 9px;border-radius:14px;border:1.5px solid rgba(255,255,255,.65);background:rgba(255,255,255,.14);color:#fff;font-weight:800;font-size:12px;letter-spacing:.04em;font-family:inherit}
.langs button.on{background:#fff;color:#0c1e30}
.langs.l2 button{color:var(--ink);border-color:var(--line);background:var(--bg);font-size:14px;padding:8px 14px}
.langs.l2 button.on{background:var(--acc);color:#fff;border-color:var(--acc)}
.tool .go .gear{font-size:22px;color:var(--ink)}
.guide{overflow-wrap:anywhere}
.guide h1{font-size:22px;margin:0 0 6px}
.guide h3{margin:18px 0 6px;color:var(--acc)}
.guide p,.guide li{line-height:1.45;font-size:15px}
.guide ul,.guide ol{padding-left:20px;margin:4px 0}
.guide li{margin-bottom:4px}
</style></head>
<body>
<header>
  <button class="brand" id="home" aria-label="Menu">
    <svg width="26" height="26" viewBox="0 0 40 40"><polygon points="20,2 36,11 36,29 20,38 4,29 4,11" fill="none" stroke="#2be06a" stroke-width="2.4"/><polygon points="20,10 29,15 29,25 20,30 11,25 11,15" fill="#2be06a" opacity=".9"/></svg>
    <div class="t">RZ <span>Tools</span></div>
  </button>
  <div id="conn"><i></i><span id="batt">--</span></div>
</header>
<main id="app"></main>
<div id="off"><div class="card"><h3 id="off_t"></h3><p class="muted" id="off_b"></p></div></div>
<div id="helpm"><div class="card"><h3 id="hm_t"></h3><p id="hm_b"></p><button class="act pri" id="hm_x">OK</button></div></div>
<script>
const $=id=>document.getElementById(id);
let cur=null,lastOk=Date.now(),view='stick',last=null;

/* ======================= LINGUE (IT / EN / FR) ======================= */
const LANGS=['it','en','fr'];
let LANG;try{LANG=localStorage.getItem('rztoolsLang')}catch(e){}
if(!LANGS.includes(LANG)){const n=(navigator.language||'it').slice(0,2).toLowerCase();LANG=LANGS.includes(n)?n:'it'}
const L={
 off_t:['Stick non collegato','Stick not connected','Stick non connecté'],
 off_b:['Sullo stick apri il menu e accendi <b>WiFi app</b>. Poi collega il telefono alla rete WiFi <b>RZTools</b> (password <b>rztools1</b>) e torna su questa pagina.','On the stick open the menu and switch on <b>WiFi app</b>. Then connect the phone to the <b>RZTools</b> WiFi network (password <b>rztools1</b>) and come back to this page.','Sur le stick, ouvrez le menu et activez <b>WiFi app</b>. Connectez ensuite le téléphone au réseau WiFi <b>RZTools</b> (mot de passe <b>rztools1</b>) et revenez sur cette page.'],
 conf_menu:['Tornare al menu? La misura in corso viene interrotta.','Back to the menu? The current measurement will be stopped.','Revenir au menu ? La mesure en cours sera interrompue.'],
 hero_sub:['Goniometro &amp; Inclinometro per superfici mobili','Protractor &amp; Inclinometer for control surfaces','Goniomètre &amp; Inclinomètre pour gouvernes'],
 start:['Inizia','Start','Commencer'],
 choose:['Scegli lo strumento','Choose the tool','Choisissez l’outil'],
 tool:['Strumento','Tool','Outil'],
 gonio:['Goniometro','Protractor','Goniomètre'],
 gonio_d:['Lettura live dell’escursione in mm e gradi, per regolare una superficie.','Live reading of the throw in mm and degrees, to set up a surface.','Lecture en direct du débattement en mm et degrés, pour régler une gouverne.'],
 incl:['Inclinometro','Inclinometer','Inclinomètre'],
 incl_d:['Confronto delle escursioni SX / DX: la SX fa da riferimento.','Compares left / right throws: the left side is the reference.','Compare les débattements G / D : le côté gauche sert de référence.'],
 settings:['Impostazioni','Settings','Réglages'],
 setup:['Setup','Setup','Réglages'],
 setup_d:['Colori dell’app, lingua, suono dello stick, informazioni.','App colors, language, stick sound, info.','Couleurs de l’app, langue, son du stick, infos.'],
 help_lab:['Aiuto','Help','Aide'],
 guide:['Guida all’uso','User guide','Guide d’utilisation'],
 guide_d:['Come si usa, hardware necessario, collegamento del telefono.','How to use it, required hardware, phone connection.','Mode d’emploi, matériel nécessaire, connexion du téléphone.'],
 distR:['Distanza R','Distance R','Distance R'],
 distR_d:['Distanza tra cerniera e bordo di uscita (mm)','Distance from hinge to trailing edge (mm)','Distance charnière – bord de fuite (mm)'],
 r_hint:['Tocca il numero per scriverlo, oppure usa − / + o lo slider. Il valore resta salvato sullo stick.','Tap the number to type it, or use − / + or the slider. The value is saved on the stick.','Touchez le nombre pour le saisir, ou utilisez − / + ou le curseur. La valeur reste enregistrée sur le stick.'],
 next:['Avanti','Next','Suivant'],
 menu_b:['‹ Menu','‹ Menu','‹ Menu'],
 g_tare:['Goniometro · Tara','Protractor · Zero','Goniomètre · Tare'],
 mount:['Monta lo stick','Mount the stick','Montez le stick'],
 g_mount_t:['Monta lo stick sulla superficie al <b>NEUTRO</b>, poi premi <b>Avvia Tara</b>.','Mount the stick on the surface at <b>NEUTRAL</b>, then press <b>Set zero</b>.','Montez le stick sur la gouverne au <b>NEUTRE</b>, puis appuyez sur <b>Tarer</b>.'],
 mount_m:['Dopo il tocco lo stick attende di essere fermo e fa la tara da solo.','After the tap the stick waits until it is still and zeroes by itself.','Après l’appui, le stick attend d’être immobile et fait la tare tout seul.'],
 start_tare:['Avvia Tara','Set zero','Tarer'],
 back_r:['‹ Imposta R','‹ Set R','‹ Régler R'],
 i_sx:['Inclinometro · Superficie SX','Inclinometer · Left surface','Inclinomètre · Gouverne gauche'],
 i_sx_t:['Posiziona lo stick sulla superficie <b>SX</b> al <b>NEUTRO</b>, poi premi <b>Avvia Tara</b>.','Place the stick on the <b>LEFT</b> surface at <b>NEUTRAL</b>, then press <b>Set zero</b>.','Placez le stick sur la gouverne <b>GAUCHE</b> au <b>NEUTRE</b>, puis appuyez sur <b>Tarer</b>.'],
 i_sx_m:['La SX fa da riferimento per il confronto.','The left side is the reference for the comparison.','Le côté gauche sert de référence pour la comparaison.'],
 i_dx:['Inclinometro · Superficie DX','Inclinometer · Right surface','Inclinomètre · Gouverne droite'],
 move:['Sposta lo stick','Move the stick','Déplacez le stick'],
 ref_ok:['Riferimento SX registrato','Left reference recorded','Référence gauche enregistrée'],
 i_dx_t:['Sposta lo stick sulla superficie <b>DX</b> al <b>NEUTRO</b>, poi premi <b>Avvia</b>.','Move the stick to the <b>RIGHT</b> surface at <b>NEUTRAL</b>, then press <b>Start</b>.','Placez le stick sur la gouverne <b>DROITE</b> au <b>NEUTRE</b>, puis appuyez sur <b>Démarrer</b>.'],
 i_dx_m:['Lo stick verifica lo zero; la tara resta quella della SX.','The stick checks the zero; the left-side zero is kept.','Le stick vérifie le zéro ; la tare du côté gauche est conservée.'],
 go:['Avvia','Start','Démarrer'],
 redo_sx:['‹ Ripeti registrazione SX','‹ Repeat left recording','‹ Refaire l’enregistrement gauche'],
 tare:['Tara','Zero','Tare'],
 check0:['Verifica zero DX','Right zero check','Vérification du zéro droit'],
 taring:['Tara in corso','Zeroing','Tare en cours'],
 hold:['Tieni fermo lo stick…','Hold the stick still…','Tenez le stick immobile…'],
 bar_m:['La barra si svuota se lo stick si muove.','The bar empties if the stick moves.','La barre se vide si le stick bouge.'],
 restart:['Ricomincia','Restart','Recommencer'],
 cancel:['‹ Annulla','‹ Cancel','‹ Annuler'],
 measure:['Misura','Measurement','Mesure'],
 retare:['Rifai tara','Zero again','Refaire la tare'],
 ref:['Riferimento','Reference','Référence'],
 move_ud:['Muovi a fondo corsa in su e in giù','Move to full travel up and down','Débattement complet en haut et en bas'],
 maxS:['Max<sub>S</sub>','Max<sub>L</sub>','Max<sub>G</sub>'],
 minS:['min<sub>S</sub>','min<sub>L</sub>','min<sub>G</sub>'],
 done:['Fine','Done','Terminer'],
 back:['‹ Indietro','‹ Back','‹ Retour'],
 zdiff:['Zero DX diverso','Right zero differs','Zéro droit différent'],
 zsub:['Regola la superficie oppure continua','Adjust the surface or continue','Réglez la gouverne ou continuez'],
 tol:['tolleranza','tolerance','tolérance'],
 cont:['Continua','Continue','Continuer'],
 repos:['‹ Riposiziona','‹ Reposition','‹ Repositionner'],
 cmp_lab:['Inclinometro · Confronto DX','Inclinometer · Right comparison','Inclinomètre · Comparaison droite'],
 cmp_big:['Regola la DX','Adjust the right side','Réglez le côté droit'],
 cmp_sub:['Porta a fondo corsa e tieni fermo 1 s per registrare','Go to full travel and hold still 1 s to record','Allez en butée et restez immobile 1 s pour enregistrer'],
 cmp_h:['Riferimento SX · registrato DX','Left reference · right recorded','Référence G · enregistré D'],
 SX:['SX','L','G'], DX:['DX','R','D'],
 result:['Risultato','Result','Résultat'],
 res_sub:['Valori in mm · tolleranza ±','Values in mm · tolerance ±','Valeurs en mm · tolérance ±'],
 diff:['Diff','Diff','Écart'],
 zrow:['Zero DX / SX','Right / left zero','Zéro D / G'],
 new_m:['Nuova misura','New measurement','Nouvelle mesure'],
 rec_arm:['Nessuna variazione: avanti tra {s} s','No change: next in {s} s','Aucune variation : suite dans {s} s'],
 rec_move:['Muovi la superficie a fondo corsa in su e in giù','Move the surface to full travel up and down','Amenez la gouverne en butée en haut et en bas'],
 z_ok:['Zero entro tolleranza: premi Continua','Zero within tolerance: press Continue','Zéro dans la tolérance : appuyez sur Continuer'],
 z_bad:['Regola la superficie finché il valore diventa verde','Adjust the surface until the value turns green','Réglez la gouverne jusqu’à ce que la valeur devienne verte'],
 hold1s:['fermo 1 s a fondo corsa per registrare','hold 1 s at full travel to record','1 s immobile en butée pour enregistrer'],
 ok:['OK','OK','OK'], out:['FUORI TOLL.','OUT OF TOL.','HORS TOL.'],
 set_sub:['Colori dell’app, lingua e stick','App colors, language and stick','Couleurs, langue et stick'],
 colors:['Colori dell’app','App colors','Couleurs de l’app'],
 bg:['Sfondo','Background','Fond'], accent:['Colore principale','Main color','Couleur principale'], barc:['Colore barra','Bar color','Couleur de la barre'],
 reset_c:['Ripristina colori','Reset colors','Réinitialiser les couleurs'],
 bg_light:['Chiaro','Light','Clair'], bg_sand:['Sabbia','Sand','Sable'], bg_dark:['Scuro','Dark','Sombre'], bg_black:['Nero','Black','Noir'],
 lang:['Lingua','Language','Langue'],
 stick:['Stick','Stick','Stick'], sound:['Suono','Sound','Son'], batt:['Batteria','Battery','Batterie'], rset:['R impostata','R set','R réglée'], ver:['Versione','Version','Version'],
 app:['App','App','App'],
 app_net:['Rete WiFi <b>RZTools</b> · password <b>rztools1</b> · indirizzo <b>192.168.4.1</b>','WiFi network <b>RZTools</b> · password <b>rztools1</b> · address <b>192.168.4.1</b>','Réseau WiFi <b>RZTools</b> · mot de passe <b>rztools1</b> · adresse <b>192.168.4.1</b>'],
 guide_btn:['📖 Guida all’uso','📖 User guide','📖 Guide d’utilisation']
};
const t=k=>{const v=L[k];return v?(v[LANGS.indexOf(LANG)]||v[0]):k};

/* ---- aiuti «?» ---- */
const HELP={
 r:[['Distanza R','R è la distanza tra la cerniera e il punto in cui vuoi conoscere l’escursione (di solito il bordo di uscita). Serve per trasformare l’angolo in mm: mm = R × sin(angolo). La posizione dello stick sulla superficie non conta. Puoi scriverla col tastierino, usare − e + oppure lo slider (5–300 mm); il valore resta salvato sullo stick.'],
    ['Distance R','R is the distance between the hinge and the point where you want to know the throw (usually the trailing edge). It turns the angle into mm: mm = R × sin(angle). Where the stick sits on the surface does not matter. Type it, use − and + or the slider (5–300 mm); the value is saved on the stick.'],
    ['Distance R','R est la distance entre la charnière et le point où vous voulez connaître le débattement (en général le bord de fuite). Elle transforme l’angle en mm : mm = R × sin(angle). La position du stick sur la gouverne n’a pas d’importance. Saisissez-la, utilisez − et + ou le curseur (5–300 mm) ; la valeur reste enregistrée sur le stick.']],
 colors:[['Colori dell’app','Scegli lo sfondo (chiaro, sabbia, scuro, nero), il colore principale (pulsanti, titoli, evidenziazioni) e il colore della barra in alto. Le scelte sono salvate su questo telefono.'],
    ['App colors','Choose the background (light, sand, dark, black), the main color (buttons, titles, highlights) and the top bar color. Choices are saved on this phone.'],
    ['Couleurs de l’app','Choisissez le fond (clair, sable, sombre, noir), la couleur principale (boutons, titres, mises en évidence) et la couleur de la barre du haut. Les choix sont enregistrés sur ce téléphone.']],
 lang:[['Lingua','Scegli la lingua dell’app: italiano, inglese o francese. È salvata su questo telefono. Lo schermo dello stick resta in italiano.'],
    ['Language','Choose the app language: Italian, English or French. It is saved on this phone. The stick screen stays in Italian.'],
    ['Langue','Choisissez la langue de l’app : italien, anglais ou français. Elle est enregistrée sur ce téléphone. L’écran du stick reste en italien.']],
 stick:[['Stick','Suono: accende o spegne i suoni dello stick (bip dei tasti, tara, registrazioni, verdetto); la scelta resta salvata sullo stick. Sotto trovi batteria, R impostata e versione del programma.'],
    ['Stick','Sound: switches the stick sounds on or off (button beeps, zero, recordings, verdict); the choice is saved on the stick. Below: battery, R set and program version.'],
    ['Stick','Son : active ou coupe les sons du stick (bips des touches, tare, enregistrements, verdict) ; le choix reste enregistré sur le stick. Dessous : batterie, R réglée et version du programme.']],
 cmp:[['Confronto DX','A sinistra il riferimento SX registrato prima, a destra il valore DX. Il valore DX viene registrato quando tieni la superficie ferma a fondo corsa per più di 1 secondo: è rosso finché non coincide con la SX entro la tolleranza, poi diventa verde.'],
    ['Right comparison','On the left the left-side reference recorded before, on the right the right-side value. The right value is recorded when you hold the surface still at full travel for more than 1 second: it is red until it matches the left side within the tolerance, then it turns green.'],
    ['Comparaison droite','À gauche la référence du côté gauche enregistrée avant, à droite la valeur du côté droit. La valeur droite est enregistrée quand vous tenez la gouverne immobile en butée plus d’une seconde : elle est rouge tant qu’elle ne correspond pas au côté gauche dans la tolérance, puis devient verte.']],
 app:[['App sul telefono','Su iPhone: Safari → Condividi → Aggiungi a Home, per avere RZ Tools come icona a schermo intero. Su Android: Chrome → menu → Aggiungi a schermata Home. Il telefono deve essere collegato alla rete WiFi RZTools dello stick (menu dello stick → WiFi app).'],
    ['App on the phone','On iPhone: Safari → Share → Add to Home Screen, to get RZ Tools as a full-screen icon. On Android: Chrome → menu → Add to Home screen. The phone must be connected to the stick’s RZTools WiFi network (stick menu → WiFi app).'],
    ['App sur le téléphone','Sur iPhone : Safari → Partager → Sur l’écran d’accueil, pour avoir RZ Tools en icône plein écran. Sur Android : Chrome → menu → Ajouter à l’écran d’accueil. Le téléphone doit être connecté au réseau WiFi RZTools du stick (menu du stick → WiFi app).']]
};
function showHelp(k){const h=HELP[k];if(!h)return;const x=h[LANGS.indexOf(LANG)]||h[0];$('hm_t').textContent=x[0];$('hm_b').textContent=x[1];$('helpm').classList.add('on')}

/* ---- guida (HTML per lingua) ---- */
const GUIDE={
it:`<h1>RZ Tools — Guida</h1>
<p>RZ Tools misura l’escursione delle superfici mobili dei modelli RC (alettoni, elevatore, flap…) con uno stick M5Stack appoggiato sulla superficie. Due strumenti: il <b>Goniometro</b> (lettura live in mm e gradi) e l’<b>Inclinometro</b> (confronto delle escursioni SX / DX). Il telefono fa da secondo schermo e telecomando.</p>
<h3>Hardware necessario</h3>
<ul>
<li><b>M5StickS3</b> (codice K150: ESP32-S3, sensore BMI270, schermo e batteria integrati) — consigliato. In alternativa <b>M5StickC Plus2</b> (sensore MPU6886).</li>
<li><b>Cavo USB-C dati</b> (non solo ricarica): per ricaricare lo stick e caricare il programma.</li>
<li><b>Slitta / culla stampata in 3D</b> con base piatta come piano di riferimento; utile un’aletta di registro sul bordo di uscita e un’apertura per l’USB-C.</li>
<li><b>Telefono</b> iPhone o Android con Safari o Chrome: l’app non si installa da uno store, è servita dallo stick.</li>
<li><b>Per caricare il programma</b>: computer con Arduino IDE, pacchetto schede <b>M5Stack</b> (URL nelle Preferenze: static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json), libreria <b>M5Unified</b>; cartella dello sketch con i tre file (.ino, web_app.h, icon_png.h).</li>
</ul>
<h3>Collegare il telefono</h3>
<ul><li>Sullo stick: menu → <b>WiFi app</b> → BLU. Compare «WiFi app: ON» con rete e indirizzo.</li>
<li>Sul telefono: rete WiFi <b>RZTools</b>, password <b>rztools1</b>, poi apri <b>http://192.168.4.1</b>.</li>
<li>iPhone: Safari → Condividi → <b>Aggiungi a Home</b>. Se iOS avvisa che la rete non ha internet, resta collegato.</li>
<li>All’accensione il WiFi è spento: consuma batteria, accendilo solo quando serve.</li></ul>
<h3>Tasti dello stick</h3>
<ul><li><b>DX</b> (frontale): indicato dalla freccia <b>in alto</b> sullo schermo, con la sua funzione.</li>
<li><b>BLU</b> (sopra): indicato dalla freccia <b>a destra</b>; «back» = indietro.</li>
<li>«tieni» = tenere premuto circa mezzo secondo. Il tasto di accensione spegne la scheda.</li></ul>
<h3>Distanza R</h3>
<p>R è la distanza tra la <b>cerniera</b> e il punto in cui vuoi l’escursione (di solito il bordo di uscita): mm = R × sin(angolo). Lo stick misura l’angolo, uguale in ogni punto della superficie: dove lo appoggi non conta. R è unica per i due strumenti e resta salvata.</p>
<h3>Goniometro</h3>
<ol><li>Imposta R → Avanti.</li><li>Monta lo stick sulla superficie al <b>neutro</b> → <b>Avvia Tara</b>: lo stick attende di essere fermo e azzera da solo.</li>
<li>Lettura live: mm grandi, gradi piccoli; il suono cresce con l’inclinazione. «Rifai tara» per riazzerare.</li></ol>
<h3>Inclinometro</h3>
<p>Uso consigliato: prima regola la superficie SX con il Goniometro, poi usa l’Inclinometro per portare la DX uguale.</p>
<ol><li>Imposta R; stick sulla <b>SX al neutro</b> → <b>Avvia Tara</b>.</li>
<li><b>Riferimento SX</b>: porta la superficie a fondo corsa in su e in giù; dopo 2 s senza variazioni la registrazione si chiude da sola («Fine» per chiudere prima).</li>
<li>Sposta lo stick sulla <b>DX al neutro</b> → <b>Avvia</b>: viene verificato lo zero (la tara resta quella della SX). Se differisce di oltre 0,5 mm compare l’allarme: regola la superficie oppure «Continua».</li>
<li><b>Confronto DX</b>: porta a fondo corsa e tieni fermo 1 s per registrare. Il valore DX è <b>rosso</b> finché non coincide con la SX entro ±0,5 mm, poi <b>verde</b>. Regola i fine corsa e ripeti; quando va bene → <b>Risultato</b>.</li>
<li><b>Risultato</b>: tabella SX / DX / differenza e, in basso, la differenza di zero (fuori tolleranza da 1 mm).</li></ol>
<h3>Consigli</h3>
<ul><li>Timone (cerniera verticale): metti il modello su un fianco, altrimenti il sensore non vede la rotazione.</li>
<li>Appoggia lo stick sempre nello stesso modo, sulla base piatta della slitta, e tienilo fermo durante tara e registrazioni.</li>
<li>Tocca il logo in alto per tornare al menu; i «?» spiegano ogni sezione.</li></ul>`,
en:`<h1>RZ Tools — Guide</h1>
<p>RZ Tools measures the throw of RC model control surfaces (ailerons, elevator, flaps…) with an M5Stack stick placed on the surface. Two tools: the <b>Protractor</b> (live reading in mm and degrees) and the <b>Inclinometer</b> (left / right throw comparison). The phone works as a second screen and remote control.</p>
<h3>Required hardware</h3>
<ul>
<li><b>M5StickS3</b> (code K150: ESP32-S3, BMI270 sensor, built-in screen and battery) — recommended. Alternatively <b>M5StickC Plus2</b> (MPU6886 sensor).</li>
<li><b>USB-C data cable</b> (not charge-only): to charge the stick and upload the program.</li>
<li><b>3D-printed sled / cradle</b> with a flat base as reference plane; a locating tab on the trailing edge and an opening for the USB-C are useful.</li>
<li><b>Phone</b>, iPhone or Android with Safari or Chrome: the app is not installed from a store, it is served by the stick.</li>
<li><b>To upload the program</b>: computer with Arduino IDE, <b>M5Stack</b> board package (URL in Preferences: static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json), <b>M5Unified</b> library; sketch folder with the three files (.ino, web_app.h, icon_png.h).</li>
</ul>
<h3>Connecting the phone</h3>
<ul><li>On the stick: menu → <b>WiFi app</b> → BLUE. «WiFi app: ON» appears with network and address.</li>
<li>On the phone: WiFi network <b>RZTools</b>, password <b>rztools1</b>, then open <b>http://192.168.4.1</b>.</li>
<li>iPhone: Safari → Share → <b>Add to Home Screen</b>. If iOS warns that the network has no internet, stay connected.</li>
<li>At power-on WiFi is off: it uses battery, switch it on only when needed.</li></ul>
<h3>Stick buttons</h3>
<ul><li><b>DX</b> (front): shown by the arrow <b>at the top</b> of the screen, with its function.</li>
<li><b>BLUE</b> (top): shown by the arrow <b>on the right</b>; «back» = go back.</li>
<li>«tieni» (hold) = keep pressed about half a second. The power button switches the board off.</li></ul>
<h3>Distance R</h3>
<p>R is the distance between the <b>hinge</b> and the point where you want the throw (usually the trailing edge): mm = R × sin(angle). The stick measures the angle, which is the same at every point of the surface: where you place it does not matter. R is shared by both tools and is saved.</p>
<h3>Protractor</h3>
<ol><li>Set R → Next.</li><li>Mount the stick on the surface at <b>neutral</b> → <b>Set zero</b>: the stick waits until it is still and zeroes by itself.</li>
<li>Live reading: large mm, small degrees; the sound rises with the angle. «Zero again» to re-zero.</li></ol>
<h3>Inclinometer</h3>
<p>Recommended: first set the left surface with the Protractor, then use the Inclinometer to make the right side match.</p>
<ol><li>Set R; stick on the <b>left surface at neutral</b> → <b>Set zero</b>.</li>
<li><b>Left reference</b>: move the surface to full travel up and down; after 2 s without changes the recording closes by itself («Done» to close earlier).</li>
<li>Move the stick to the <b>right surface at neutral</b> → <b>Start</b>: the zero is checked (the left-side zero is kept). If it differs by more than 0.5 mm an alarm appears: adjust the surface or «Continue».</li>
<li><b>Right comparison</b>: go to full travel and hold still 1 s to record. The right value is <b>red</b> until it matches the left within ±0.5 mm, then <b>green</b>. Adjust the end points and repeat; when it is right → <b>Result</b>.</li>
<li><b>Result</b>: left / right / difference table and, at the bottom, the zero difference (out of tolerance from 1 mm).</li></ol>
<h3>Tips</h3>
<ul><li>Rudder (vertical hinge): lay the model on its side, otherwise the sensor cannot see the rotation.</li>
<li>Always place the stick the same way, on the flat base of the sled, and keep it still while zeroing and recording.</li>
<li>Tap the logo at the top to go back to the menu; the «?» buttons explain each section.</li></ul>`,
fr:`<h1>RZ Tools — Guide</h1>
<p>RZ Tools mesure le débattement des gouvernes des modèles RC (ailerons, profondeur, volets…) avec un stick M5Stack posé sur la gouverne. Deux outils : le <b>Goniomètre</b> (lecture en direct en mm et degrés) et l’<b>Inclinomètre</b> (comparaison des débattements gauche / droite). Le téléphone sert de second écran et de télécommande.</p>
<h3>Matériel nécessaire</h3>
<ul>
<li><b>M5StickS3</b> (code K150 : ESP32-S3, capteur BMI270, écran et batterie intégrés) — conseillé. Sinon <b>M5StickC Plus2</b> (capteur MPU6886).</li>
<li><b>Câble USB-C de données</b> (pas seulement de charge) : pour recharger le stick et charger le programme.</li>
<li><b>Support / berceau imprimé en 3D</b> avec une base plate comme plan de référence ; une butée sur le bord de fuite et une ouverture pour l’USB-C sont utiles.</li>
<li><b>Téléphone</b> iPhone ou Android avec Safari ou Chrome : l’app ne s’installe pas depuis un store, elle est servie par le stick.</li>
<li><b>Pour charger le programme</b> : ordinateur avec Arduino IDE, paquet de cartes <b>M5Stack</b> (URL dans les Préférences : static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json), bibliothèque <b>M5Unified</b> ; dossier du sketch avec les trois fichiers (.ino, web_app.h, icon_png.h).</li>
</ul>
<h3>Connecter le téléphone</h3>
<ul><li>Sur le stick : menu → <b>WiFi app</b> → BLEU. «WiFi app: ON» s’affiche avec le réseau et l’adresse.</li>
<li>Sur le téléphone : réseau WiFi <b>RZTools</b>, mot de passe <b>rztools1</b>, puis ouvrez <b>http://192.168.4.1</b>.</li>
<li>iPhone : Safari → Partager → <b>Sur l’écran d’accueil</b>. Si iOS signale que le réseau n’a pas d’internet, restez connecté.</li>
<li>À l’allumage le WiFi est coupé : il consomme de la batterie, activez-le seulement si besoin.</li></ul>
<h3>Touches du stick</h3>
<ul><li><b>DX</b> (avant) : indiquée par la flèche <b>en haut</b> de l’écran, avec sa fonction.</li>
<li><b>BLEU</b> (dessus) : indiquée par la flèche <b>à droite</b> ; «back» = retour.</li>
<li>«tieni» (maintenir) = appuyer environ une demi-seconde. Le bouton d’alimentation éteint la carte.</li></ul>
<h3>Distance R</h3>
<p>R est la distance entre la <b>charnière</b> et le point où vous voulez le débattement (en général le bord de fuite) : mm = R × sin(angle). Le stick mesure l’angle, identique en tout point de la gouverne : sa position n’a pas d’importance. R est commune aux deux outils et reste enregistrée.</p>
<h3>Goniomètre</h3>
<ol><li>Réglez R → Suivant.</li><li>Montez le stick sur la gouverne au <b>neutre</b> → <b>Tarer</b> : le stick attend d’être immobile et fait la tare tout seul.</li>
<li>Lecture en direct : mm en grand, degrés en petit ; le son monte avec l’angle. «Refaire la tare» pour remettre à zéro.</li></ol>
<h3>Inclinomètre</h3>
<p>Conseil : réglez d’abord la gouverne gauche avec le Goniomètre, puis utilisez l’Inclinomètre pour aligner le côté droit.</p>
<ol><li>Réglez R ; stick sur la <b>gouverne gauche au neutre</b> → <b>Tarer</b>.</li>
<li><b>Référence gauche</b> : amenez la gouverne en butée en haut et en bas ; après 2 s sans variation l’enregistrement se ferme tout seul («Terminer» pour fermer avant).</li>
<li>Placez le stick sur la <b>gouverne droite au neutre</b> → <b>Démarrer</b> : le zéro est vérifié (la tare gauche est conservée). S’il diffère de plus de 0,5 mm une alarme s’affiche : réglez la gouverne ou «Continuer».</li>
<li><b>Comparaison droite</b> : allez en butée et restez immobile 1 s pour enregistrer. La valeur droite est <b>rouge</b> tant qu’elle ne correspond pas à la gauche à ±0,5 mm près, puis <b>verte</b>. Réglez les fins de course et recommencez ; quand c’est bon → <b>Résultat</b>.</li>
<li><b>Résultat</b> : tableau gauche / droite / écart et, en bas, l’écart de zéro (hors tolérance à partir de 1 mm).</li></ol>
<h3>Conseils</h3>
<ul><li>Dérive (charnière verticale) : posez le modèle sur le côté, sinon le capteur ne voit pas la rotation.</li>
<li>Posez toujours le stick de la même façon, sur la base plate du support, et gardez-le immobile pendant la tare et les enregistrements.</li>
<li>Touchez le logo en haut pour revenir au menu ; les «?» expliquent chaque section.</li></ul>`
};

/* ======================= COLORI (come RZ Ballast) ======================= */
const BGS={light:{bg:'#e9eff5',card:'#ffffff',ink:'#0f2233',muted:'#6b7f93',line:'#e3eaf1'},sand:{bg:'#f2ebdf',card:'#fffdf8',ink:'#2b2418',muted:'#8a7d68',line:'#e8dfce'},dark:{bg:'#06111b',card:'#102030',ink:'#e8f0f7',muted:'#8da3b8',line:'#1d3145'},black:{bg:'#000000',card:'#151515',ink:'#f2f2f2',muted:'#9a9a9a',line:'#2a2a2a'}};
const ACCS=['#19b956','#0b7bd6','#f08a00','#e0405a','#8e44ad','#00a5b5'];
const BARS={blue:['#0c1e30','#14405f'],char:['#161616','#3a3a3a'],forest:['#0d2b1f','#1c5a3f'],wine:['#2a0f18','#5a1f33'],navy:['#0b2a6f','#1565c0']};
const TH0={bg:'light',acc:'#19b956',bar:'blue'};
let TH;try{TH=Object.assign({},TH0,JSON.parse(localStorage.getItem('rztoolsTheme')||'{}'))}catch(e){TH=Object.assign({},TH0)}
function applyTheme(){const r=document.documentElement.style,b=BGS[TH.bg]||BGS.light,h=BARS[TH.bar]||BARS.blue;
  r.setProperty('--bg',b.bg);r.setProperty('--card',b.card);r.setProperty('--ink',b.ink);r.setProperty('--muted',b.muted);r.setProperty('--line',b.line);
  r.setProperty('--acc',TH.acc);r.setProperty('--h1',h[0]);r.setProperty('--h2',h[1]);
  r.colorScheme=(TH.bg==='dark'||TH.bg==='black')?'dark':'light';
  const mt=document.querySelector('meta[name=theme-color]');if(mt)mt.content=h[0];
  try{localStorage.setItem('rztoolsTheme',JSON.stringify(TH))}catch(e){}
  if($('swbg')){
    $('swbg').innerHTML=Object.keys(BGS).map(k=>`<button class="sw bgsw ${TH.bg===k?'on':''}" data-bg="${k}" style="background:${BGS[k].bg};color:${BGS[k].ink}">${t('bg_'+k)}</button>`).join('');
    $('swacc').innerHTML=ACCS.map(c=>`<button class="sw ${TH.acc===c?'on':''}" data-acc="${c}" style="background:${c}"></button>`).join('');
    $('swbar').innerHTML=Object.keys(BARS).map(k=>`<button class="sw ${TH.bar===k?'on':''}" data-bar="${k}" style="background:linear-gradient(135deg,${BARS[k][0]},${BARS[k][1]})"></button>`).join('')}}

const langsHTML=cls=>`<div class="langs ${cls}">${[['it','Italiano','IT'],['en','English','EN'],['fr','Français','FR']].map(x=>`<button data-lang="${x[0]}" class="${LANG===x[0]?'on':''}">${cls==='l2'?x[1]:x[2]}</button>`).join('')}</div>`;
function setLang(l){LANG=l;try{localStorage.setItem('rztoolsLang',l)}catch(e){}document.documentElement.lang=l;staticText();
  if(view==='setup')openSetup();else if(view==='guide')openGuide();else{cur=null;if(last)apply(last)}}
function staticText(){$('off_t').textContent=t('off_t');$('off_b').innerHTML=t('off_b')}

document.addEventListener('click',e=>{const b=e.target.closest('button');if(!b)return;
  if(b.dataset.bg){TH.bg=b.dataset.bg;applyTheme()}else if(b.dataset.acc){TH.acc=b.dataset.acc;applyTheme()}else if(b.dataset.bar){TH.bar=b.dataset.bar;applyTheme()}
  else if(b.dataset.lang){setLang(b.dataset.lang)}
  else if(b.classList.contains('q')){showHelp(b.dataset.h)}});
$('hm_x').onclick=()=>$('helpm').classList.remove('on');

/* ======================= COLLEGAMENTO CON LO STICK ======================= */
const f1=x=>{x=Math.round(x*10)/10;if(Object.is(x,-0))x=0;return (x>0?'+':'')+x.toFixed(1)};
function cmd(k,v){fetch('/c?k='+k+(v!==undefined?'&v='+encodeURIComponent(v):''),{cache:'no-store'})
  .then(r=>r.json()).then(apply).catch(()=>{})}
function poll(){fetch('/s',{cache:'no-store'}).then(r=>r.json()).then(j=>{lastOk=Date.now();apply(j)})
  .catch(()=>{}).finally(()=>setTimeout(poll,200))}
setInterval(()=>{const on=Date.now()-lastOk<1500;$('conn').classList.toggle('on',on);$('off').classList.toggle('on',!on&&view==='stick')},300);

$('home').onclick=()=>{if(view!=='stick'){closePage();return}
  if(!cur||cur==='MENU'||cur==='LOGO')return;
  if(confirm(t('conf_menu')))cmd('menu')};

/* ======================= SCHERMATE ======================= */
const B=(id,cls,txt,k)=>`<button class="act ${cls}" id="${id}" data-k="${k}">${txt}</button>`;
const live=()=>`<div class="cgbox"><div class="v"><span id="mm">--</span> <small>mm</small></div><div class="d"><span id="dg">--</span>°</div></div>`;
const head=(lab,big,sub)=>`<div class="card mtop"><div class="mlab">${lab}</div><div class="mbig">${big}</div>${sub?`<div class="muted">${sub}</div>`:''}</div>`;
const card=(id,lab,big,txt,icon)=>`<button class="card tool" id="${id}"><span class="go">${icon||'›'}</span><div class="mlab">${lab}</div><div class="mbig">${big}</div><div class="muted">${txt}</div></button>`;
const V={
LOGO:()=>`<div class="hero">${langsHTML('l1')}<h1>RZ <span>Tools</span></h1><p>${t('hero_sub')}</p></div>
  <div class="btns">${B('b1','pri',t('start'),'dx')}</div>`,
MENU:()=>`<div class="hero">${langsHTML('l1')}<h1>RZ <span>Tools</span></h1><p>${t('choose')}</p></div>
  <button class="card tool" data-k="gonio"><span class="go">›</span><div class="mlab">${t('tool')}</div><div class="mbig">${t('gonio')}</div><div class="muted">${t('gonio_d')}</div></button>
  <button class="card tool" data-k="incl"><span class="go">›</span><div class="mlab">${t('tool')}</div><div class="mbig">${t('incl')}</div><div class="muted">${t('incl_d')}</div></button>
  ${card('opset',t('settings'),t('setup'),t('setup_d'),'<span class="gear">⚙︎</span>')}
  ${card('opgd',t('help_lab'),t('guide'),t('guide_d'),'<span class="gear">📖</span>')}`,
SETUP_R:j=>`${head(j.p?t('incl'):t('gonio'),t('distR'),t('distR_d'))}
  <div class="card"><h2 class="hq">R (mm) <button class="q" data-h="r">?</button></h2><div class="rrow"><button id="rm">−</button><input id="rv" type="number" inputmode="numeric" min="1" max="2000" value="${Math.round(j.R)}"><button id="rp">+</button></div>
  <input type="range" id="rs" min="5" max="300" step="1" value="${Math.min(300,Math.max(5,Math.round(j.R)))}"><div class="rlim"><span>5</span><span>300 mm</span></div>
  <p class="muted" style="margin:10px 0 0">${t('r_hint')}</p></div>
  <div class="btns">${B('b1','pri',t('next'),'setr')}${B('b2','',t('menu_b'),'menu')}</div>`,
G_MOUNT:j=>`${head(t('g_tare'),t('mount'),'R = '+Math.round(j.R)+' mm')}
  <div class="card"><p class="txt">${t('g_mount_t')}</p><p class="muted">${t('mount_m')}</p></div>
  <div class="btns">${B('b1','pri',t('start_tare'),'dx')}${B('b2','',t('back_r'),'blu')}</div>`,
I_MOUNT_SX:j=>`${head(t('i_sx'),t('mount'),'R = '+Math.round(j.R)+' mm')}
  <div class="card"><p class="txt">${t('i_sx_t')}</p><p class="muted">${t('i_sx_m')}</p></div>
  <div class="btns">${B('b1','pri',t('start_tare'),'dx')}${B('b2','',t('back_r'),'blu')}</div>`,
I_MOUNT_DX:j=>`${head(t('i_dx'),t('move'),t('ref_ok'))}
  <div class="card"><p class="txt">${t('i_dx_t')}</p><p class="muted">${t('i_dx_m')}</p></div>
  <div class="btns">${B('b1','pri',t('go'),'dx')}${B('b2','',t('redo_sx'),'blu')}</div>`,
STAB_RUN:j=>`${head(j.sm?t('i_dx'):t('tare'),j.sm?t('check0'):t('taring'),'')}
  <div class="card"><p class="txt">${t('hold')}</p><div class="bar"><i id="pb"></i></div><p class="muted">${t('bar_m')}</p></div>
  <div class="btns">${B('b1','',t('restart'),'dx')}${B('b2','',t('cancel'),'blu')}</div>`,
G_LIVE:j=>`${head(t('gonio'),t('measure'),'R = <span id="rr"></span> mm')}
  <div class="card">${live()}</div>
  <div class="btns">${B('b1','pri',t('retare'),'dx')}${B('b2','',t('back_r'),'blu')}</div>`,
I_REC:j=>`${head(t('i_sx'),t('ref'),t('move_ud'))}
  <div class="card">${live()}<div class="stat"><div><span>${t('maxS')}</span><b id="mx">--</b></div><div><span>${t('minS')}</span><b id="mn">--</b></div></div>
  <p class="muted" id="rs2" style="margin:12px 0 0;text-align:center"></p></div>
  <div class="btns">${B('b1','pri',t('done'),'dx')}${B('b2','',t('back'),'blu')}</div>`,
I_ZERO_WARN:j=>`${head(t('i_dx'),t('zdiff'),t('zsub'))}
  <div class="card"><div class="cgbox"><div class="v" id="zv"><span id="z">--</span> <small>mm</small></div><div class="d">${t('tol')} ±<span id="zt"></span> mm</div></div>
  <p class="muted" id="zs" style="margin:12px 0 0;text-align:center"></p></div>
  <div class="btns">${B('b1','pri',t('cont'),'dx')}${B('b2','',t('repos'),'blu')}</div>`,
I_CMP:j=>`${head(t('cmp_lab'),t('cmp_big'),t('cmp_sub'))}
  <div class="card">${live()}</div>
  <div class="card"><h2 class="hq">${t('cmp_h')} <button class="q" data-h="cmp">?</button></h2>
   <div class="cmp"><div class="k">Max</div><div class="c"><span>${t('SX')}</span><b id="ms">--</b></div><div class="c" id="cd1"><span>${t('DX')}</span><b id="md">--</b></div><div class="df" id="f1"></div></div>
   <div class="cmp"><div class="k">min</div><div class="c"><span>${t('SX')}</span><b id="ns">--</b></div><div class="c" id="cd2"><span>${t('DX')}</span><b id="nd">--</b></div><div class="df" id="f2"></div></div></div>
  <div class="btns">${B('b1','pri',t('result'),'dx')}${B('b2','',t('back'),'blu')}</div>`,
I_RESULT:j=>`${head(t('incl'),t('result'),t('res_sub')+j.tol.toFixed(1)+' mm')}
  <div class="card"><table><tr><th></th><th>${t('SX')}</th><th>${t('DX')}</th><th>${t('diff')}</th></tr>
   <tr><td>Max</td><td id="r1"></td><td id="r2"></td><td><span class="st" id="r3"></span></td></tr>
   <tr><td>min</td><td id="r4"></td><td id="r5"></td><td><span class="st" id="r6"></span></td></tr></table>
   <div class="zrow"><span class="muted">${t('zrow')}</span><span class="st" id="zr"></span></div></div>
  <div class="btns">${B('b1','pri',t('new_m'),'dx')}${B('b2','',t('menu_b'),'blu')}</div>`
};

function build(j){const f=V[j.s];$('app').innerHTML=f?f(j):'';window.scrollTo(0,0);
  document.querySelectorAll('#app [data-k]').forEach(b=>b.onclick=()=>{const k=b.dataset.k;
    if(k==='setr'){const v=parseInt($('rv').value,10);if(!(v>=1)){$('rv').focus();return}cmd('setr',v)}else cmd(k)});
  if(j.s==='SETUP_R'){const i=$('rv'),sl=$('rs');
    const sync=()=>{sl.value=Math.min(300,Math.max(5,parseInt(i.value,10)||5))};
    const step=d=>{i.value=Math.max(1,(parseInt(i.value,10)||0)+d);sync()};
    $('rm').onclick=()=>step(-1);$('rp').onclick=()=>step(1);
    i.oninput=sync;sl.oninput=()=>{i.value=sl.value}}
  if($('opset'))$('opset').onclick=openSetup;
  if($('opgd'))$('opgd').onclick=openGuide}

/* ---- pagine solo del telefono: Setup e Guida (lo stick resta dov'è) ---- */
function setupHTML(){return `${head(t('settings'),t('setup'),t('set_sub'))}
  <div class="card"><h2 class="hq">${t('colors')} <button class="q" data-h="colors">?</button></h2>
    <div class="muted">${t('bg')}</div><div id="swbg" style="margin:6px 0 10px"></div>
    <div class="muted">${t('accent')}</div><div id="swacc" style="margin:6px 0 10px"></div>
    <div class="muted">${t('barc')}</div><div id="swbar" style="margin:6px 0 4px"></div>
    <button class="act" id="thres" style="margin-top:10px">${t('reset_c')}</button></div>
  <div class="card"><h2 class="hq">${t('lang')} <button class="q" data-h="lang">?</button></h2>${langsHTML('l2')}</div>
  <div class="card"><h2 class="hq">${t('stick')} <button class="q" data-h="stick">?</button></h2>
    <div class="set"><span>${t('sound')}</span><button class="tg" id="sndt"></button></div>
    <div class="set"><span>${t('batt')}</span><b id="sb">--</b></div>
    <div class="set"><span>${t('rset')}</span><b id="sr">--</b></div>
    <div class="set"><span>${t('ver')}</span><b id="sv">--</b></div></div>
  <div class="card"><h2 class="hq">${t('app')} <button class="q" data-h="app">?</button></h2>
    <div class="muted">${t('app_net')}</div>
    <button class="act" id="gd2" style="margin-top:12px">${t('guide_btn')}</button></div>
  <div class="btns">${B('sxb','',t('back'),'')}</div>`}
function openSetup(){view='setup';$('app').innerHTML=setupHTML();window.scrollTo(0,0);applyTheme();
  $('thres').onclick=()=>{TH=Object.assign({},TH0);applyTheme()};
  $('sndt').onclick=()=>{const on=!(last&&last.snd);cmd('snd',on?1:0)};
  $('gd2').onclick=openGuide;$('sxb').onclick=closePage;if(last)updSetup(last)}
function updSetup(j){const g=$('sndt');if(g)g.classList.toggle('on',!!j.snd);
  T('sb',j.b>=0?j.b+'%':'--');T('sr',Math.round(j.R)+' mm');T('sv','v'+(j.ver||'?'))}
function openGuide(){view='guide';$('app').innerHTML=`<div class="card guide">${GUIDE[LANG]||GUIDE.it}</div><div class="btns">${B('gxb','',t('back'),'')}</div>`;
  window.scrollTo(0,0);$('gxb').onclick=closePage}
function closePage(){view='stick';cur=null;if(last)apply(last);else $('app').innerHTML=''}

/* ======================= AGGIORNAMENTO VALORI ======================= */
const T=(id,x)=>{const e=$(id);if(e)e.textContent=x};
const okc=(e,ok)=>{if(e){e.classList.toggle('ok',ok);e.classList.toggle('bad',!ok)}};
function apply(j){last=j;$('batt').textContent=j.b>=0?j.b+'%':'--';
  if(view==='setup'){updSetup(j);return}
  if(view!=='stick')return;
  if(j.s!==cur){cur=j.s;build(j)}
  T('mm',f1(j.mm));T('dg',f1(j.dg));T('rr',Math.round(j.R));
  const pb=$('pb');if(pb)pb.style.width=Math.round(j.sf*100)+'%';
  if(j.s==='I_REC'){T('mx',f1(j.rx));T('mn',f1(j.rn));
    T('rs2',j.arm?t('rec_arm').replace('{s}',j.left.toFixed(1)):t('rec_move'))}
  if(j.s==='I_ZERO_WARN'){T('z',f1(j.z));T('zt',j.zt.toFixed(1));const ok=Math.abs(j.z)<=j.zt;okc($('zv'),ok);T('zs',ok?t('z_ok'):t('z_bad'))}
  if(j.s==='I_CMP'){T('ms',f1(j.ms));T('ns',f1(j.ns));
    const row=(has,dv,ref,cid,vid,fid)=>{const c=$(cid);if(!has){T(vid,'--');c.classList.add('bad');c.classList.remove('ok');T(fid,t('hold1s'));return}
      T(vid,f1(dv));const d=Math.abs(dv)-Math.abs(ref);okc(c,Math.abs(d)<=j.tol);T(fid,t('diff')+' '+f1(d)+' mm')};
    row(j.hm,j.md,j.ms,'cd1','md','f1');row(j.hn,j.nd,j.ns,'cd2','nd','f2')}
  if(j.s==='I_RESULT'){T('r1',f1(j.ms));T('r2',f1(j.md));T('r4',f1(j.ns));T('r5',f1(j.nd));
    const d1=Math.abs(Math.abs(j.ms)-Math.abs(j.md)),d2=Math.abs(Math.abs(j.ns)-Math.abs(j.nd));
    T('r3',d1.toFixed(1));okc($('r3'),d1<=j.tol);T('r6',d2.toFixed(1));okc($('r6'),d2<=j.tol);
    const zok=Math.abs(j.z)<j.zf;T('zr',f1(j.z)+' mm '+(zok?t('ok'):t('out')));okc($('zr'),zok)}
}
document.documentElement.lang=LANG;staticText();applyTheme();poll();
</script></body></html>
)RZHTML";
