#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<meta name="theme-color" content="#000000">
<title>Zuno</title>
<style>
  :root{
    --bg:#000; --card:#fff; --pink:#ff3b5c; --text:#fff; --dark:#111; --muted:#9ca3af; --line:#2a2a2a;
  }
  *{box-sizing:border-box;}
  body{margin:20px;font-family:-apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif;background:var(--bg);color:var(--text);padding-bottom:40px;}
  .wrap{padding:0 16px;max-width:480px;margin:0 auto;}

  header{display:flex;align-items:center;gap:10px;padding:0 0 14px;}
  header .logo-wrap{width:44px;height:44px;flex:none;}
  header h1{font-size:30px;margin:0;color:var(--pink);font-weight:800;letter-spacing:.01em;}
  header .battBadge{margin-left:auto;font-size:13px;color:var(--muted);font-weight:700;}

  .card{background:var(--card);color:var(--dark);border-radius:18px;padding:16px;margin-bottom:18px;}
  .card .row{display:flex;justify-content:space-between;align-items:center;font-size:11px;font-weight:700;color:#555;text-transform:uppercase;letter-spacing:.03em;}

  h2{font-size:12px;color:var(--muted);text-transform:uppercase;letter-spacing:.06em;margin:25px 0 8px;}

  .grid3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;}
  .grid2{display:grid;grid-template-columns:1fr 1fr;gap:10px;}
  .button-group{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;}
  .btn{ background:#fff;
        color:#111;
        border:1px solid #333;
        border-radius:12px;
        padding:14px 6px;
        font-size:14px;
        font-weight:800;
        text-align:center;
        letter-spacing:.02em;
        }

.btn.pinkAction.btn:active {
    background: var(--card);
    color: #000;
    border-color: var(--card);
}
.btn.active {
    background: var(--pink);
    color: #fff;
    border-color: var(--pink);
        }
.btn.iconbtn.btn:active {
    background: var(--pink);
    color: #ffffff;
    border-color: var(--pink);
}

  .btn.wide{grid-column:1/-1;}
  .btn.pinkAction{background:var(--pink);color:#fff;border-color:var(--pink);}
  .btn.iconbtn{background:#fff;color:#111;font-size:30px;padding:14px 0;}
  .btn.ghost{background:var(--pink);color:#fff;border:1px solid #333;}

  .dpad-row{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;margin-bottom:10px; align-items:center; justify-content:center;}
  .settingInput{background:#fff;color:#111;border:1px solid #333;border-radius:10px;padding:12px 10px;font-family:inherit;font-size:14px;box-sizing:border-box;width:100%;}
  .toast{position:fixed;bottom:16px;left:16px;right:16px;background:#161616;border:1px solid #333;border-radius:10px;padding:12px 14px;font-size:14px;display:none;max-width:480px;margin:0 auto;color:#fff;}

  /* ---- Setup / WELCOME screen (shown while in the setup hotspot) ---- */
  #setupScreen{display:none;text-align:center;padding-top:8vh;}
  #setupScreen .setupLogo{width:180px;height:180px;margin:0 auto 10px;}
  #setupScreen .setupWordmark{font-size:54px;color:var(--pink);font-weight:800;letter-spacing:.01em;margin:0 0 12vh;}
  #setupScreen h1.welcome{font-size:30px;color:#fff;margin:0 0 24px;letter-spacing:.02em;}
  #setupScreen .setupField{text-align:left;margin:0 auto 16px;max-width:340px;}
  #setupScreen .setupField label{display:block;font-size:12px;font-weight:800;color:#fff;letter-spacing:.05em;margin-bottom:6px;}
  #setupScreen .setupActions{max-width:340px;margin:20px auto 0;}

  /* ---- Settings panel (hidden behind the SETTINGS button) ---- */
  #settingsPanel{display:none;}
  #settingsPanel h2:first-child{margin-top:0;}

  /* ---- Games panel + individual game screens ---- */
  #gamesPanel{display:none;}
  #ticTacToeScreen, #catchMeScreen{display:none;text-align:center;}
  .tttGrid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:260px;margin:0 auto 16px;}
  .tttCell{background:#fff;color:#111;font-size:32px;font-weight:800;height:80px;display:flex;align-items:center;justify-content:center;border-radius:12px;cursor:pointer;user-select:none;}
  .gameNarrow{max-width:260px;margin:0 auto;}
</style>
</head>
<body>

  <!-- ============ WELCOME / SETUP SCREEN (shown only while the robot is
       in its own setup hotspot, with no home WiFi saved yet) ============ -->
  <div id="setupScreen" class="wrap">
    <svg class="setupLogo" viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
      <circle cx="30" cy="15" r="6" fill="#a78bfa"/><circle cx="70" cy="15" r="6" fill="#a78bfa"/>
      <line x1="30" y1="15" x2="30" y2="28" stroke="#a78bfa" stroke-width="3"/>
      <line x1="70" y1="15" x2="70" y2="28" stroke="#a78bfa" stroke-width="3"/>
      <rect x="10" y="28" width="80" height="62" rx="26" fill="#f5f5f7"/>
      <rect x="26" y="40" width="48" height="38" rx="14" fill="#6d5ce8"/>
      <circle cx="40" cy="59" r="5" fill="#fff"/><circle cx="60" cy="59" r="5" fill="#fff"/>
      <path d="M42 68 Q50 74 58 68" stroke="#fff" stroke-width="3" fill="none" stroke-linecap="round"/>
    </svg>
    <div class="setupWordmark">zuno</div>
    <h1 class="welcome">WELCOME</h1>
    <div class="setupField">
      <label>SSID</label>
      <input type="text" id="setupSsid" class="settingInput" placeholder="Your WiFi name">
    </div>
    <div class="setupField">
      <label>PASSWORD</label>
      <input type="password" id="setupPass" class="settingInput" placeholder="Your WiFi password">
    </div>
    <div class="setupActions">
      <div class="btn pinkAction wide" onclick="setWifi()">CONNECT WIFI</div>
    </div>
  </div>

  <!-- ============ MAIN DASHBOARD (shown once connected to home WiFi) ============ -->
  <div id="dashboard">
    <header>
      <div class="logo-wrap"><svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
        <circle cx="30" cy="15" r="6" fill="#a78bfa"/><circle cx="70" cy="15" r="6" fill="#a78bfa"/>
        <line x1="30" y1="15" x2="30" y2="28" stroke="#a78bfa" stroke-width="3"/>
        <line x1="70" y1="15" x2="70" y2="28" stroke="#a78bfa" stroke-width="3"/>
        <rect x="10" y="28" width="80" height="62" rx="26" fill="#f5f5f7"/>
        <rect x="26" y="40" width="48" height="38" rx="14" fill="#6d5ce8"/>
        <circle cx="40" cy="59" r="5" fill="#fff"/><circle cx="60" cy="59" r="5" fill="#fff"/>
        <path d="M42 68 Q50 74 58 68" stroke="#fff" stroke-width="3" fill="none" stroke-linecap="round"/>
      </svg></div>
      <h1>zuno</h1>
      <span class="battBadge" id="battBadge"></span>
    </header>
    <h2>Status</h2>
    <div class="button-group">
      <div class="btn pinkAction" onclick="showWeather()">WEATHER</div>
      <div class="btn pinkAction" onclick="sendCmd('scan')">SCAN</div>
      <div class="btn pinkAction" onclick="sendCmd('show_temp')">TEMP</div>
    </div>

    <h2>Mode</h2>
    <div class="button-group" id="modeGrid">
      <div class="btn" data-mode="self_driving" onclick="selectMode('self_driving')">SELF DRIVING</div>
      <div class="btn" data-mode="still" onclick="selectMode('still')">STILL</div>
      <div class="btn" data-mode="manual" onclick="selectMode('manual')">MANUAL</div>
    </div>

    <h2>Controls</h2>
    <div class="dpad-row">
      <div></div>
      <div class="btn iconbtn" onclick="dpad('forward')">&#9650;</div>
      <div></div>
    </div>
    <div class="dpad-row">
      <div class="btn iconbtn" onclick="dpad('left')">&#9664;</div>
      <div class="btn iconbtn" onclick="dpad('stop')">&#9209;</div>
      <div class="btn iconbtn" onclick="dpad('right')">&#9654;</div>
    </div>
    <div class="dpad-row">
      <div></div>
      <div class="btn iconbtn" onclick="dpad('backward')">&#9660;</div>
      <div></div>
    </div>

    <div class="btn ghost wide" onclick="toggleSettings()" style="margin-top:60px;">&#9881; SETTINGS</div>

    <div id="settingsPanel" class="card">
      <h2>Timezone</h2>
      <div class="row"><span>UTC OFFSET (HOURS)</span></div>
      <input type="number" id="gmtOffset" step="0.25" class="settingInput" style="margin:8px 0;">

      <h2>Default Weather Location</h2>
      <div class="row"><span>USED WHEN THE PHONE DOESN'T SHARE GPS</span></div>
      <div class="grid2" style="margin:8px 0;">
        <input type="number" id="defLat" step="0.0001" placeholder="Latitude" class="settingInput">
        <input type="number" id="defLon" step="0.0001" placeholder="Longitude" class="settingInput">
      </div>
      <div class="btn pinkAction wide" onclick="saveSettings()">SAVE SETTINGS</div>

      <h2>WiFi</h2>
      <div class="row"><span>CONNECT TO A DIFFERENT NETWORK</span></div>
      <input type="text" id="wifiSsid" class="settingInput" placeholder="WiFi name" style="margin:8px 0;">
      <input type="password" id="wifiPass" class="settingInput" placeholder="WiFi password" style="margin:0 0 8px;">
      <div class="btn pinkAction wide" onclick="connectNewWifi()">CONNECT WIFI</div>
      <div class="btn ghost wide" onclick="resetWifi()" style="margin-top:10px;">RESET WIFI</div>
    </div>

    <div class="btn ghost wide" onclick="toggleGames()" style="margin-top:10px; margin-bottom:20px;">&#127918; GAMES</div>

    <div id="gamesPanel" class="card">
      <div class="btn pinkAction wide" onclick="openGame('ttt')">TIC TAC TOE</div>
      <div class="btn pinkAction wide" onclick="openGame('catch')" style="margin-top:10px;">CATCH ME</div>
    </div>
  </div>

  <!-- ============ TIC TAC TOE (played on Zuno's OLED - this is just the controller) ============ -->
  <div id="ticTacToeScreen" class="wrap">
    <h2 style="margin-top:24px;">Tic Tac Toe</h2>
    <div style="font-size:12px;color:var(--muted);margin-bottom:10px;">Board is on Zuno's screen - tap here to play</div>
    <div id="tttStatus" style="margin-bottom:14px;font-weight:800;font-size:15px;">Your turn</div>
    <div class="tttGrid" id="tttBoard"></div>
    <div class="grid2 gameNarrow">
      <div class="btn pinkAction" onclick="tttNewGame()">NEW GAME</div>
      <div class="btn ghost" onclick="closeGame()">BACK</div>
    </div>
  </div>

  <!-- ============ CATCH ME ============ -->
  <div id="catchMeScreen" class="wrap">
    <h2 style="margin-top:24px;">Catch Me</h2>
    <div id="catchStatus" style="font-size:14px;color:var(--muted);margin-bottom:18px;">Zuno will scoot around avoiding walls - cover its front sensor with your hand to catch it.</div>
    <div id="catchTimer" style="font-size:48px;font-weight:800;color:var(--pink);margin-bottom:22px;">0s</div>
    <div class="grid2 gameNarrow">
      <div class="btn pinkAction" onclick="catchStart()">START</div>
      <div class="btn ghost" onclick="catchStop()">STOP</div>
    </div>
    <div class="btn ghost wide gameNarrow" onclick="closeGame()" style="margin-top:14px;">BACK</div>
  </div>

<div class="toast" id="toast"></div>

<script>

    document.querySelectorAll('.button-group').forEach(group => {
    const buttons = group.querySelectorAll('.btn');

    buttons.forEach(button => {
        button.addEventListener('click', () => {
            buttons.forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');
        });
    });
});

function toast(msg){
  const t=document.getElementById('toast');
  t.textContent=msg; t.style.display='block';
  clearTimeout(t._h); t._h=setTimeout(()=>t.style.display='none',3000);
}

function toggleSettings(){
  const panel = document.getElementById('settingsPanel');
  panel.style.display = (panel.style.display === 'block') ? 'none' : 'block';
}

function toggleGames(){
  const panel = document.getElementById('gamesPanel');
  panel.style.display = (panel.style.display === 'block') ? 'none' : 'block';
}

let knownApMode = null;

async function refreshStatus(){
  try{
    const r = await fetch('/api/status');
    const d = await r.json();

    if (knownApMode !== d.ap_mode){
      knownApMode = d.ap_mode;
      document.getElementById('setupScreen').style.display = d.ap_mode ? 'block' : 'none';
      document.getElementById('dashboard').style.display = d.ap_mode ? 'none' : 'block';
    }
    if (d.ap_mode) return;

    document.querySelectorAll('#modeGrid .btn').forEach(b=>{
      b.classList.toggle('active', b.dataset.mode === d.mode);
    });

    const battEl = document.getElementById('battBadge');
    if (battEl) {
      if (d.battery_pct != null && d.battery_pct >= 0) {
        const v = (d.battery_v != null) ? ' (' + d.battery_v.toFixed(1) + 'V)' : '';
        battEl.textContent = '\uD83D\uDD0B ' + d.battery_pct + '%' + v;
        battEl.style.color = (d.battery_pct <= 20) ? 'var(--pink)' : 'var(--muted)';
      } else {
        battEl.textContent = '';
      }
    }
  }catch(e){ /* robot unreachable - silently retry on the next poll */ }
}

async function sendCmd(cmd){
  try{
    const r = await fetch('/api/command',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({cmd})});
    const d = await r.json();
    toast(d.message || 'Done');
    refreshStatus();
  }catch(e){ toast('Command failed'); }
}

function selectMode(mode){ sendCmd('mode_' + mode); }
function dpad(dir){ sendCmd('manual_' + dir); }

function showWeather(){
  if(!navigator.geolocation){ toast('No GPS access - using default location'); fetchWeatherAt(null,null); return; }
  toast('Getting your location...');
  navigator.geolocation.getCurrentPosition(
    pos => fetchWeatherAt(pos.coords.latitude, pos.coords.longitude),
    err => { toast('Location unavailable - using default location'); fetchWeatherAt(null,null); }
  );
}

async function fetchWeatherAt(lat, lon){
  try{
    const body = (lat!=null && lon!=null) ? {lat, lon} : {};
    const r = await fetch('/api/weather',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
    const d = await r.json();
    toast(d.message || (d.ok ? 'Weather shown on display' : 'Weather sync failed'));
  }catch(e){ toast('Weather sync failed'); }
}

// Used by the WELCOME/setup screen (first-time provisioning, robot in AP mode).
async function setWifi(){
  const ssid=document.getElementById('setupSsid').value.trim();
  const pass=document.getElementById('setupPass').value;
  if(!ssid){ toast('Enter a WiFi name'); return; }
  try{
    const r = await fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,password:pass})});
    const d = await r.json();
    toast(d.message || 'Saving...');
  }catch(e){ toast('Could not reach robot'); }
}

// Used by the Settings panel's WiFi section - lets you point the robot at
// a different network at any time, without needing to reset into AP mode.
async function connectNewWifi(){
  const ssid=document.getElementById('wifiSsid').value.trim();
  const pass=document.getElementById('wifiPass').value;
  if(!ssid){ toast('Enter a WiFi name'); return; }
  try{
    const r = await fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,password:pass})});
    const d = await r.json();
    toast(d.message || 'Saving...');
  }catch(e){ toast('Could not reach robot'); }
}

async function resetWifi(){
  if(!confirm('Reset WiFi settings? The robot will forget this network and restart its setup hotspot.')) return;
  try{
    const r = await fetch('/api/wifi/reset',{method:'POST'});
    const d = await r.json();
    toast(d.message || 'Resetting...');
  }catch(e){ toast('Could not reach robot'); }
}

async function loadSettings(){
  try{
    const r = await fetch('/api/settings');
    const d = await r.json();
    document.getElementById('gmtOffset').value = d.gmt_offset;
    document.getElementById('defLat').value = d.lat;
    document.getElementById('defLon').value = d.lon;
  }catch(e){ /* leave fields blank if unreachable */ }
}

async function saveSettings(){
  const gmt_offset = parseFloat(document.getElementById('gmtOffset').value);
  const lat = parseFloat(document.getElementById('defLat').value);
  const lon = parseFloat(document.getElementById('defLon').value);
  if(isNaN(gmt_offset) || isNaN(lat) || isNaN(lon)){ toast('Fill in all settings fields'); return; }
  try{
    const r = await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({gmt_offset, lat, lon})});
    const d = await r.json();
    toast(d.message || 'Settings saved');
  }catch(e){ toast('Could not save settings'); }
}

// ---- Game navigation ----
let catchPollTimer = null;
let tttPollTimer = null;

function openGame(which){
  document.getElementById('dashboard').style.display = 'none';
  if (which === 'ttt') {
    document.getElementById('ticTacToeScreen').style.display = 'block';
    tttNewGame();
    if (tttPollTimer) clearInterval(tttPollTimer);
    tttPollTimer = setInterval(tttSyncStatus, 2000);
  } else if (which === 'catch') {
    document.getElementById('catchMeScreen').style.display = 'block';
  }
}

function closeGame(){
  const wasCatch = document.getElementById('catchMeScreen').style.display === 'block';
  const wasTtt = document.getElementById('ticTacToeScreen').style.display === 'block';
  document.getElementById('ticTacToeScreen').style.display = 'none';
  document.getElementById('catchMeScreen').style.display = 'none';
  document.getElementById('dashboard').style.display = 'block';
  if (catchPollTimer) { clearInterval(catchPollTimer); catchPollTimer = null; }
  if (tttPollTimer) { clearInterval(tttPollTimer); tttPollTimer = null; }
  if (wasCatch) sendCmd('mode_still'); // stop the robot if leaving mid-game
  if (wasTtt) fetch('/api/ttt/stop', {method:'POST'}).catch(()=>{});
}

// ---- Tic Tac Toe: the board is drawn on Zuno's OLED - this screen is
// just the controller. Tapping a cell posts the move to the robot; the
// mirrored grid here is synced from /api/status for visual feedback.
let tttBoard = '         ';

function tttRenderBoard(){
  const el = document.getElementById('tttBoard');
  el.innerHTML = '';
  for (let i = 0; i < 9; i++) {
    const cell = document.createElement('div');
    cell.className = 'tttCell';
    cell.textContent = (tttBoard[i] === ' ') ? '' : tttBoard[i];
    cell.onclick = () => tttPlay(i);
    el.appendChild(cell);
  }
}

async function tttSyncStatus(){
  try{
    const r = await fetch('/api/status');
    const d = await r.json();
    const t = d.ttt || {};
    tttBoard = t.board || '         ';
    tttRenderBoard();
    const status = document.getElementById('tttStatus');
    if (t.result === 'player')     status.textContent = 'You win!';
    else if (t.result === 'robot') status.textContent = 'Zuno wins!';
    else if (t.result === 'draw')  status.textContent = "It's a draw!";
    else status.textContent = t.player_turn ? 'Your turn' : "Zuno's turn...";
  }catch(e){ /* keep trying on the next poll */ }
}

async function tttNewGame(){
  try{
    const r = await fetch('/api/ttt/start', {method:'POST'});
    const d = await r.json();
    toast(d.message || 'Game started');
  }catch(e){ toast('Could not reach robot'); }
  tttSyncStatus();
}

async function tttPlay(i){
  if (tttBoard[i] !== ' ') return;
  try{
    const r = await fetch('/api/ttt/move', {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({cell:i})});
    const d = await r.json();
    if (!d.ok) toast(d.message || 'Invalid move');
  }catch(e){ toast('Move failed'); }
  tttSyncStatus();
}

// ---- Catch Me (robot evades, polls /api/status for the live timer) ----
function catchStart(){
  sendCmd('mode_catch_me');
  document.getElementById('catchStatus').textContent = "Zuno is on the run - go catch it!";
  document.getElementById('catchTimer').textContent = '0s';
  if (catchPollTimer) clearInterval(catchPollTimer);
  catchPollTimer = setInterval(catchPoll, 500);
}

function catchStop(){
  sendCmd('mode_still');
  if (catchPollTimer) { clearInterval(catchPollTimer); catchPollTimer = null; }
  document.getElementById('catchTimer').textContent = '0s';
  document.getElementById('catchStatus').textContent = "Zuno will scoot around avoiding walls - cover its front sensor with your hand to catch it.";
}

async function catchPoll(){
  try{
    const r = await fetch('/api/status');
    const d = await r.json();
    const c = d.catch_me || {};
    if (c.state === 'running') {
      document.getElementById('catchTimer').textContent = (c.elapsed_sec || 0) + 's';
    } else if (c.state === 'caught') {
      document.getElementById('catchTimer').textContent = (c.elapsed_sec || 0) + 's';
      document.getElementById('catchStatus').textContent = 'Caught! Nice reflexes.';
      if (catchPollTimer) { clearInterval(catchPollTimer); catchPollTimer = null; }
    }
  }catch(e){ /* keep trying on the next poll */ }
}

refreshStatus();
loadSettings();
setInterval(refreshStatus, 2000);
</script>
</body>
</html>

)rawliteral";
