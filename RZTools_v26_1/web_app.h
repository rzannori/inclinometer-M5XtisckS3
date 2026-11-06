// ============================================================================
//  web_app.h  -  pagina web dell'app RZ Tools per telefono (servita dallo stick)
//  Stile ripreso da RZ Ballast Archive. Tutto in un file: HTML + CSS + JS.
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
</style></head><body>
<header>
  <button class="brand" id="home" aria-label="Menu">
    <svg width="26" height="26" viewBox="0 0 40 40"><polygon points="20,2 36,11 36,29 20,38 4,29 4,11" fill="none" stroke="#2be06a" stroke-width="2.4"/><polygon points="20,10 29,15 29,25 20,30 11,25 11,15" fill="#2be06a" opacity=".9"/></svg>
    <div class="t">RZ <span>Tools</span></div>
  </button>
  <div id="conn"><i></i><span id="batt">--</span></div>
</header>
<main id="app"></main>
<div id="off"><div class="card"><h3>Stick non collegato</h3>
  <p class="muted">Sullo stick apri il menu e accendi <b>WiFi app</b>. Poi collega il telefono alla rete WiFi <b>RZTools</b> (password <b>rztools1</b>) e torna su questa pagina.</p></div></div>
<div id="helpm"><div class="card"><h3 id="hm_t"></h3><p id="hm_b"></p><button class="act pri" id="hm_x">OK</button></div></div>
<script>
const $=id=>document.getElementById(id);
let cur=null,lastOk=Date.now(),view='stick',last=null;

/* ---- colori dell'app (come RZ Ballast), salvati sul telefono ---- */
const BGS={light:{n:'Chiaro',bg:'#e9eff5',card:'#ffffff',ink:'#0f2233',muted:'#6b7f93',line:'#e3eaf1'},sand:{n:'Sabbia',bg:'#f2ebdf',card:'#fffdf8',ink:'#2b2418',muted:'#8a7d68',line:'#e8dfce'},dark:{n:'Scuro',bg:'#06111b',card:'#102030',ink:'#e8f0f7',muted:'#8da3b8',line:'#1d3145'},black:{n:'Nero',bg:'#000000',card:'#151515',ink:'#f2f2f2',muted:'#9a9a9a',line:'#2a2a2a'}};
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
    $('swbg').innerHTML=Object.keys(BGS).map(k=>`<button class="sw bgsw ${TH.bg===k?'on':''}" data-bg="${k}" style="background:${BGS[k].bg};color:${BGS[k].ink}">${BGS[k].n}</button>`).join('');
    $('swacc').innerHTML=ACCS.map(c=>`<button class="sw ${TH.acc===c?'on':''}" data-acc="${c}" style="background:${c}"></button>`).join('');
    $('swbar').innerHTML=Object.keys(BARS).map(k=>`<button class="sw ${TH.bar===k?'on':''}" data-bar="${k}" style="background:linear-gradient(135deg,${BARS[k][0]},${BARS[k][1]})"></button>`).join('')}}

/* ---- aiuti «?» ---- */
const HELP={
 r:['Distanza R',"R è la distanza tra la cerniera e il punto in cui vuoi conoscere l'escursione (di solito il bordo di uscita). Serve per trasformare l'angolo in mm: mm = R × sin(angolo). La posizione dello stick sulla superficie non conta. Puoi scriverla col tastierino, usare − e + oppure lo slider (5–300 mm); il valore resta salvato sullo stick."],
 colors:['Colori dell\'app',"Scegli lo sfondo (chiaro, sabbia, scuro, nero), il colore principale (pulsanti, titoli, evidenziazioni) e il colore della barra in alto. Le scelte sono salvate su questo telefono."],
 stick:['Stick',"Suono: accende o spegne i suoni dello stick (bip dei tasti, tara, registrazioni, verdetto). La scelta resta salvata sullo stick. Sotto trovi batteria, R impostata e versione del programma."],
 cmp:['Confronto DX',"A sinistra il riferimento SX registrato prima, a destra il valore DX. Il valore DX viene registrato quando tieni la superficie ferma a fondo corsa per più di 1 secondo: è rosso finché non coincide con la SX entro la tolleranza, poi diventa verde."],
 app:['App sul telefono',"Su iPhone: Safari → Condividi → Aggiungi a Home, per avere RZ Tools come icona a schermo intero. Il telefono deve essere collegato alla rete WiFi RZTools dello stick (menu dello stick → WiFi app)."]
};
function showHelp(k){const h=HELP[k];if(!h)return;$('hm_t').textContent=h[0];$('hm_b').textContent=h[1];$('helpm').classList.add('on')}
document.addEventListener('click',e=>{const b=e.target.closest('button');if(!b)return;
  if(b.dataset.bg){TH.bg=b.dataset.bg;applyTheme()}else if(b.dataset.acc){TH.acc=b.dataset.acc;applyTheme()}else if(b.dataset.bar){TH.bar=b.dataset.bar;applyTheme()}
  else if(b.classList.contains('q')){showHelp(b.dataset.h)}});
$('hm_x').onclick=()=>$('helpm').classList.remove('on');
applyTheme();
const f1=x=>{x=Math.round(x*10)/10;if(Object.is(x,-0))x=0;return (x>0?'+':'')+x.toFixed(1)};
const a1=x=>Math.abs(Math.round(x*10)/10).toFixed(1);

function cmd(k,v){fetch('/c?k='+k+(v!==undefined?'&v='+encodeURIComponent(v):''),{cache:'no-store'})
  .then(r=>r.json()).then(apply).catch(()=>{})}
function poll(){fetch('/s',{cache:'no-store'}).then(r=>r.json()).then(j=>{lastOk=Date.now();apply(j)})
  .catch(()=>{}).finally(()=>setTimeout(poll,200))}
setInterval(()=>{const on=Date.now()-lastOk<1500;$('conn').classList.toggle('on',on);$('off').classList.toggle('on',!on)},300);

$('home').onclick=()=>{if(view==='setup'){closeSetup();return}
  if(!cur||cur==='MENU'||cur==='LOGO')return;
  if(confirm('Tornare al menu? La misura in corso viene interrotta.'))cmd('menu')};

/* ---- costruttori delle schermate (una volta per schermata) ---- */
const B=(id,cls,txt,k)=>`<button class="act ${cls}" id="${id}" data-k="${k}">${txt}</button>`;
const live=`<div class="cgbox"><div class="v"><span id="mm">--</span> <small>mm</small></div><div class="d"><span id="dg">--</span>°</div></div>`;
const head=(lab,big,sub)=>`<div class="card mtop"><div class="mlab">${lab}</div><div class="mbig">${big}</div>${sub?`<div class="muted">${sub}</div>`:''}</div>`;
const V={
LOGO:()=>`<div class="hero"><h1>RZ <span>Tools</span></h1><p>Goniometro &amp; Inclinometro per superfici mobili</p></div>
  <div class="btns">${B('b1','pri','Inizia','dx')}</div>`,
MENU:()=>`<div class="hero"><h1>RZ <span>Tools</span></h1><p>Scegli lo strumento</p></div>
  <button class="card tool" data-k="gonio"><span class="go">›</span><div class="mlab">Strumento</div><div class="mbig">Goniometro</div><div class="muted">Lettura live dell'escursione in mm e gradi, per regolare una superficie.</div></button>
  <button class="card tool" data-k="incl"><span class="go">›</span><div class="mlab">Strumento</div><div class="mbig">Inclinometro</div><div class="muted">Confronto delle escursioni SX / DX: la SX fa da riferimento.</div></button>
  <button class="card tool" id="opset"><span class="gear">⚙︎</span><div class="mlab">Impostazioni</div><div class="mbig">Setup</div><div class="muted">Colori dell'app, suono dello stick, informazioni.</div></button>`,
SETUP_R:j=>`${head(j.p?'Inclinometro':'Goniometro','Distanza R','Distanza tra cerniera e bordo di uscita (mm)')}
  <div class="card"><h2 class="hq">R (mm) <button class="q" data-h="r">?</button></h2><div class="rrow"><button id="rm">−</button><input id="rv" type="number" inputmode="numeric" min="1" max="2000" value="${Math.round(j.R)}"><button id="rp">+</button></div>
  <input type="range" id="rs" min="5" max="300" step="1" value="${Math.min(300,Math.max(5,Math.round(j.R)))}"><div class="rlim"><span>5</span><span>300 mm</span></div>
  <p class="muted" style="margin:10px 0 0">Tocca il numero per scriverlo, oppure usa − / + o lo slider. Il valore resta salvato sullo stick.</p></div>
  <div class="btns">${B('b1','pri','Avanti','setr')}${B('b2','','‹ Menu','menu')}</div>`,
G_MOUNT:j=>`${head('Goniometro · Tara','Monta lo stick','R = '+Math.round(j.R)+' mm')}
  <div class="card"><p class="txt">Monta lo stick sulla superficie al <b>NEUTRO</b>, poi premi <b>Avvia Tara</b>.</p><p class="muted">Dopo il tocco lo stick attende di essere fermo e fa la tara da solo.</p></div>
  <div class="btns">${B('b1','pri','Avvia Tara','dx')}${B('b2','','‹ Imposta R','blu')}</div>`,
I_MOUNT_SX:j=>`${head('Inclinometro · Superficie SX','Monta lo stick','R = '+Math.round(j.R)+' mm')}
  <div class="card"><p class="txt">Posiziona lo stick sulla superficie <b>SX</b> al <b>NEUTRO</b>, poi premi <b>Avvia Tara</b>.</p><p class="muted">La SX fa da riferimento per il confronto.</p></div>
  <div class="btns">${B('b1','pri','Avvia Tara','dx')}${B('b2','','‹ Imposta R','blu')}</div>`,
I_MOUNT_DX:j=>`${head('Inclinometro · Superficie DX','Sposta lo stick','Riferimento SX registrato')}
  <div class="card"><p class="txt">Sposta lo stick sulla superficie <b>DX</b> al <b>NEUTRO</b>, poi premi <b>Avvia</b>.</p><p class="muted">Lo stick verifica lo zero; la tara resta quella della SX.</p></div>
  <div class="btns">${B('b1','pri','Avvia','dx')}${B('b2','','‹ Ripeti registrazione SX','blu')}</div>`,
STAB_RUN:j=>`${head(j.sm?'Inclinometro · Superficie DX':'Tara',j.sm?'Verifica zero DX':'Tara in corso','')}
  <div class="card"><p class="txt">Tieni fermo lo stick…</p><div class="bar"><i id="pb"></i></div><p class="muted">La barra si svuota se lo stick si muove.</p></div>
  <div class="btns">${B('b1','','Ricomincia','dx')}${B('b2','','‹ Annulla','blu')}</div>`,
G_LIVE:j=>`${head('Goniometro','Misura','R = <span id="rr"></span> mm')}
  <div class="card">${live}</div>
  <div class="btns">${B('b1','pri','Rifai tara','dx')}${B('b2','','‹ Imposta R','blu')}</div>`,
I_REC:j=>`${head('Inclinometro · Superficie SX','Riferimento','Muovi a fondo corsa in su e in giù')}
  <div class="card">${live}<div class="stat"><div><span>Max<sub>S</sub></span><b id="mx">--</b></div><div><span>min<sub>S</sub></span><b id="mn">--</b></div></div>
  <p class="muted" id="rs" style="margin:12px 0 0;text-align:center"></p></div>
  <div class="btns">${B('b1','pri','Fine','dx')}${B('b2','','‹ Indietro','blu')}</div>`,
I_ZERO_WARN:j=>`${head('Inclinometro · Superficie DX','Zero DX diverso','Regola la superficie oppure continua')}
  <div class="card"><div class="cgbox"><div class="v" id="zv"><span id="z">--</span> <small>mm</small></div><div class="d">tolleranza ±<span id="zt"></span> mm</div></div>
  <p class="muted" id="zs" style="margin:12px 0 0;text-align:center"></p></div>
  <div class="btns">${B('b1','pri','Continua','dx')}${B('b2','','‹ Riposiziona','blu')}</div>`,
I_CMP:j=>`${head('Inclinometro · Confronto DX','Regola la DX','Porta a fondo corsa e tieni fermo 1 s per registrare')}
  <div class="card">${live}</div>
  <div class="card"><h2 class="hq">Riferimento SX · registrato DX <button class="q" data-h="cmp">?</button></h2>
   <div class="cmp"><div class="k">Max</div><div class="c"><span>SX</span><b id="ms">--</b></div><div class="c" id="cd1"><span>DX</span><b id="md">--</b></div><div class="df" id="f1"></div></div>
   <div class="cmp"><div class="k">min</div><div class="c"><span>SX</span><b id="ns">--</b></div><div class="c" id="cd2"><span>DX</span><b id="nd">--</b></div><div class="df" id="f2"></div></div></div>
  <div class="btns">${B('b1','pri','Risultato','dx')}${B('b2','','‹ Indietro','blu')}</div>`,
I_RESULT:j=>`${head('Inclinometro','Risultato','Valori in mm · tolleranza ±'+j.tol.toFixed(1)+' mm')}
  <div class="card"><table><tr><th></th><th>SX</th><th>DX</th><th>Diff</th></tr>
   <tr><td>Max</td><td id="r1"></td><td id="r2"></td><td><span class="st" id="r3"></span></td></tr>
   <tr><td>min</td><td id="r4"></td><td id="r5"></td><td><span class="st" id="r6"></span></td></tr></table>
   <div class="zrow"><span class="muted">Zero DX / SX</span><span class="st" id="zr"></span></div></div>
  <div class="btns">${B('b1','pri','Nuova misura','dx')}${B('b2','','‹ Menu','blu')}</div>`
};

function build(j){const f=V[j.s];$('app').innerHTML=f?f(j):'';window.scrollTo(0,0);
  document.querySelectorAll('[data-k]').forEach(b=>b.onclick=()=>{const k=b.dataset.k;
    if(k==='setr'){const v=parseInt($('rv').value,10);if(!(v>=1)){$('rv').focus();return}cmd('setr',v)}else cmd(k)});
  if(j.s==='SETUP_R'){const i=$('rv'),sl=$('rs');
    const sync=()=>{sl.value=Math.min(300,Math.max(5,parseInt(i.value,10)||5))};
    const step=d=>{i.value=Math.max(1,(parseInt(i.value,10)||0)+d);sync()};
    $('rm').onclick=()=>step(-1);$('rp').onclick=()=>step(1);
    i.oninput=sync;sl.oninput=()=>{i.value=sl.value}}
  if($('opset'))$('opset').onclick=openSetup}

/* ---- pagina Setup (solo sul telefono: lo stick resta dov'è) ---- */
function setupHTML(){const j=last||{};return `${head('Impostazioni','Setup','Colori dell\'app e stick')}
  <div class="card"><h2 class="hq">Colori dell'app <button class="q" data-h="colors">?</button></h2>
    <div class="muted">Sfondo</div><div id="swbg" style="margin:6px 0 10px"></div>
    <div class="muted">Colore principale</div><div id="swacc" style="margin:6px 0 10px"></div>
    <div class="muted">Colore barra</div><div id="swbar" style="margin:6px 0 4px"></div>
    <button class="act" id="thres" style="margin-top:10px">Ripristina colori</button></div>
  <div class="card"><h2 class="hq">Stick <button class="q" data-h="stick">?</button></h2>
    <div class="set"><span>Suono</span><button class="tg" id="sndt"></button></div>
    <div class="set"><span>Batteria</span><b id="sb">--</b></div>
    <div class="set"><span>R impostata</span><b id="sr">--</b></div>
    <div class="set"><span>Versione</span><b id="sv">--</b></div></div>
  <div class="card"><h2 class="hq">App <button class="q" data-h="app">?</button></h2>
    <div class="muted">Rete WiFi <b>RZTools</b> · password <b>rztools1</b> · indirizzo <b>192.168.4.1</b></div></div>
  <div class="btns">${B('sxb','','‹ Indietro','')}</div>`}
function openSetup(){view='setup';$('app').innerHTML=setupHTML();window.scrollTo(0,0);applyTheme();
  $('thres').onclick=()=>{TH=Object.assign({},TH0);applyTheme()};
  $('sndt').onclick=()=>{const on=!(last&&last.snd);cmd('snd',on?1:0)};
  $('sxb').onclick=closeSetup;if(last)updSetup(last)}
function closeSetup(){view='stick';cur=null;if(last)apply(last)}
function updSetup(j){const t=$('sndt');if(t)t.classList.toggle('on',!!j.snd);
  T('sb',j.b>=0?j.b+'%':'--');T('sr',Math.round(j.R)+' mm');T('sv','v'+(j.ver||'?'))}

/* ---- aggiornamento dei valori (a ogni lettura) ---- */
const T=(id,t)=>{const e=$(id);if(e)e.textContent=t};
const okc=(e,ok)=>{if(e){e.classList.toggle('ok',ok);e.classList.toggle('bad',!ok)}};
function apply(j){last=j;$('batt').textContent=j.b>=0?j.b+'%':'--';
  if(view==='setup'){updSetup(j);return}
  if(j.s!==cur){cur=j.s;build(j)}
  T('mm',f1(j.mm));T('dg',f1(j.dg));T('rr',Math.round(j.R));
  const pb=$('pb');if(pb)pb.style.width=Math.round(j.sf*100)+'%';
  if(j.s==='I_REC'){T('mx',f1(j.rx));T('mn',f1(j.rn));
    T('rs',j.arm?('Nessuna variazione: avanti tra '+j.left.toFixed(1)+' s'):'Muovi la superficie a fondo corsa in su e in giù')}
  if(j.s==='I_ZERO_WARN'){T('z',f1(j.z));T('zt',j.zt.toFixed(1));const ok=Math.abs(j.z)<=j.zt;okc($('zv'),ok);
    T('zs',ok?'Zero entro tolleranza: premi Continua':'Regola la superficie finché il valore diventa verde')}
  if(j.s==='I_CMP'){T('ms',f1(j.ms));T('ns',f1(j.ns));
    const row=(has,dv,ref,cid,vid,fid)=>{const c=$(cid);if(!has){T(vid,'--');c.classList.add('bad');c.classList.remove('ok');T(fid,'fermo 1 s a fondo corsa per registrare');return}
      T(vid,f1(dv));const d=Math.abs(dv)-Math.abs(ref);okc(c,Math.abs(d)<=j.tol);T(fid,'diff '+f1(d)+' mm')};
    row(j.hm,j.md,j.ms,'cd1','md','f1');row(j.hn,j.nd,j.ns,'cd2','nd','f2')}
  if(j.s==='I_RESULT'){T('r1',f1(j.ms));T('r2',f1(j.md));T('r4',f1(j.ns));T('r5',f1(j.nd));
    const d1=Math.abs(Math.abs(j.ms)-Math.abs(j.md)),d2=Math.abs(Math.abs(j.ns)-Math.abs(j.nd));
    T('r3',d1.toFixed(1));okc($('r3'),d1<=j.tol);T('r6',d2.toFixed(1));okc($('r6'),d2<=j.tol);
    const zok=Math.abs(j.z)<j.zf;T('zr',f1(j.z)+' mm '+(zok?'OK':'FUORI TOLL.'));okc($('zr'),zok)}
}
poll();
</script></body></html>)RZHTML";
