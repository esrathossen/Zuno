const $=id=>document.getElementById(id);
async function post(url){try{await fetch(url,{method:'POST'});await refresh();}catch(e){}}
document.querySelectorAll('[data-post]').forEach(b=>b.addEventListener('click',()=>post(b.dataset.post)));
async function refresh(){
  try{
    const r=await fetch('/api/status',{cache:'no-store'});
    const s=await r.json();
    $('statusLine').textContent=(s.wifi?'WiFi ready':'WiFi offline')+' | '+(s.obstacle?'Obstacle':'Path clear');
    $('battery').textContent=s.battery+'%';
    $('voltage').textContent=Number(s.voltage).toFixed(2)+'V';
    $('mode').textContent=s.mode;
    $('face').textContent=s.face;
    $('distance').textContent=s.distance+' cm';
    $('motor').textContent=s.motor;
    $('temp').textContent=s.dhtValid?Number(s.temperature).toFixed(1)+' C':'--';
    $('humidity').textContent=s.dhtValid?Number(s.humidity).toFixed(1)+'%':'--';
    $('game').textContent=s.game+' | '+s.gameStatus;
  }catch(e){$('statusLine').textContent='Reconnecting...';}
}
setInterval(refresh,1000);
refresh();
