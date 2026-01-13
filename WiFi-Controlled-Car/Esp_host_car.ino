#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <map>
#define RXD2 16
#define TXD2 17

// ====== CONFIG ======
const char* ssid = "Crazy_Techies_Rover";  
const char* password = "12345678";         
const char* hostPassword = "admin123";     

WebServer server(80);
DNSServer dnsServer;

std::map<String, String> activeUsers; // ip -> last action
bool gestureEnabled = false;

// ====== HTML PAGES ======
const char CONTROL_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8'>
<title>Rover Control</title>
<link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@700&display=swap" rel="stylesheet">
<style>
  body { font-family: Arial, sans-serif; text-align: center; background: #f2f2f2; }
  .container { border: 5px solid #333; border-radius: 20px; padding: 30px;
    max-width: 600px; margin: 30px auto; background: #fff; box-shadow: 0 4px 12px rgba(0,0,0,0.2); }
  .team-title { font-size: 70px; margin-bottom: 20px; font-weight: 700;
    font-family: 'Orbitron', sans-serif; color: #222; letter-spacing: 4px; text-transform: uppercase; }
  .rover-title { font-size: 40px; margin-bottom: 40px; font-weight: 700;
    font-family: 'Orbitron', sans-serif; color: #111; letter-spacing: 6px; text-transform: uppercase; }
  .btn { display: inline-flex; justify-content: center; align-items: center;
    width: 130px; height: 130px; border-radius: 50%; font-size: 22px; font-weight: bold; margin: 10px;
    background: #4CAF50; color: white; border: none; cursor: pointer; transition: transform 0.1s ease-in-out; }
  .btn:active { background: #45a049; transform: scale(0.95); }
  .stop-btn { background: #f44336; font-size: 20px; }
  .grid { display: grid; grid-template-columns: repeat(3, 1fr);
    grid-gap: 15px; justify-items: center; align-items: center; max-width: 450px; margin: auto; }
  .note { margin-top: 20px; font-size: 18px; color: #333; }
  #hostBtn { margin-top: 25px; font-size: 18px; padding: 10px 20px; background: orange; border:none; border-radius:10px; cursor:pointer; }
</style>
<script>
  function sendCmd(cmd) {
    fetch("/cmd?c=" + cmd);
  }
  function hostLogin() {
    window.location.href="/hostlogin";
  }
  document.addEventListener("keydown", function(event) {
    let key = event.key.toLowerCase();
    if (key === "w") sendCmd("F");
    if (key === "a") sendCmd("L");
    if (key === "s") sendCmd("B");
    if (key === "d") sendCmd("R");
    if (key === " ") sendCmd("S");
  });
</script>
</head>
<body>
  <div class="container">
    <div class="team-title">Crazy Techies</div>
    <div class="rover-title">ROVER</div>
    <div class='grid'>
      <div></div>
      <button class='btn' onclick="sendCmd('F')">^</button>
      <div></div>

      <button class='btn' onclick="sendCmd('L')"><</button>
      <button class='btn stop-btn' onclick="sendCmd('S')">■</button>
      <button class='btn' onclick="sendCmd('R')">></button>

      <div></div>
      <button class='btn' onclick="sendCmd('B')">v</button>
      <div></div>
    </div>
    <div class="note">Use W / A / S / D keys and Spacebar for control 🚀</div>
    <button id="hostBtn" onclick="hostLogin()">🔑 Host</button>
  </div>
</body>
</html>
)rawliteral";

const char HOST_LOGIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>Host Login</title>
<style>body{font-family:Arial;text-align:center;margin-top:60px}</style></head>
<body>
  <h1>Host Panel Login</h1>
  <form method="GET" action="/host">
    <input type="password" name="pw" placeholder="Password" style="font-size:18px;padding:8px;" />
    <button type="submit" style="font-size:18px;padding:8px 16px;">Enter</button>
  </form>
</body></html>
)rawliteral";

const char HOST_PANEL_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8"><title>Host Panel - Crazy Techies</title>
<style>
  body{font-family:Arial;text-align:center;background:#f2f2f2;}
  table{margin:auto;border-collapse:collapse;width:90%;}
  th,td{border:1px solid #333;padding:10px;text-align:center;}
  th{background:#333;color:#fff;}
  button{padding:8px 14px;border:none;border-radius:8px;cursor:pointer;}
  .kick{background:red;color:white;}
  .gesture{background:#2196F3;color:white;margin-top:15px;}
</style>
<script>
  async function loadUsers(){
    let res = await fetch("/users");
    let data = await res.json();
    let tbl = "<tr><th>User IP</th><th>Last Action</th><th>Kick</th></tr>";
    for(let u of data){
      tbl += "<tr><td>"+u.ip+"</td><td>"+u.action+"</td>"+
             "<td><button class='kick' onclick='kickUser(\""+u.ip+"\")'>Kick</button></td></tr>";
    }
    document.getElementById("usertable").innerHTML = tbl;
  }
  function kickUser(ip){
    fetch("/kick?ip="+ip).then(loadUsers);
  }
  function toggleGesture(){
    fetch("/gesture").then(r=>r.text()).then(alert);
  }
  setInterval(loadUsers,2000);
</script>
</head>
<body onload="loadUsers()">
  <h1>👑 Crazy Techies Host Panel 👑</h1>
  <table id="usertable"></table>
  <button class="gesture" onclick="toggleGesture()">Toggle Gesture</button>
</body>
</html>
)rawliteral";

// ====== ROUTES ======
void handleRoot(){ server.send(200,"text/html",CONTROL_PAGE); }
void handleHostLogin(){ server.send(200,"text/html",HOST_LOGIN_PAGE); }
void handleHostAuth(){
  if(server.hasArg("pw") && server.arg("pw")==hostPassword){
    server.send(200,"text/html",HOST_PANEL_PAGE);
  } else {
    server.send(200,"text/html","<h1>Wrong Password</h1><a href='/hostlogin'>Try Again</a>");
  }
}
void handleCommand(){
  if(server.hasArg("c")){
    String cmd=server.arg("c");
    String ip=server.client().remoteIP().toString();
    activeUsers[ip]=cmd;
    Serial2.println(cmd);
    // send to motors here
    server.send(200,"text/plain","OK");
  }
}
void handleUsers(){
  String json="[";
  bool first=true;
  for(auto &u:activeUsers){
    if(!first) json+=",";
    json+="{\"ip\":\""+u.first+"\",\"action\":\""+u.second+"\"}";
    first=false;
  }
  json+="]";
  server.send(200,"application/json",json);
}
void handleKick(){
  if(server.hasArg("ip")){
    String ip=server.arg("ip");
    activeUsers.erase(ip);
    server.send(200,"text/plain","User "+ip+" kicked");
  } else server.send(200,"text/plain","No IP given");
}
void handleGesture(){
  gestureEnabled=!gestureEnabled;
  server.send(200,"text/plain",gestureEnabled?"Gesture Enabled":"Gesture Disabled");
}

// ====== SETUP ======
void setup(){
  Serial.begin(115200);
  WiFi.softAP(ssid,password);
  dnsServer.start(53,"*",WiFi.softAPIP());

  server.on("/",handleRoot);
  server.on("/hostlogin",handleHostLogin);
  server.on("/host",handleHostAuth);
  server.on("/cmd",handleCommand);
  server.on("/users",handleUsers);
  server.on("/kick",handleKick);
  server.on("/gesture",handleGesture);

  server.begin();
  Serial.println("AP Started!");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  Serial.print("Portal IP: "); Serial.println(WiFi.softAPIP());
}
void loop(){
  dnsServer.processNextRequest();
  server.handleClient();
}
