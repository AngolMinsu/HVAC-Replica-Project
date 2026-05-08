#include "WebUi.h"

#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "CanDriver.h"
#include "GDS.h"

struct HuWebState {
  uint8_t power;
  uint8_t fan;
  uint8_t temp;
  uint8_t wind;
  uint8_t ac;
  uint8_t autoMode;
  uint8_t screen;
  uint8_t media;
  uint8_t volume;
  uint8_t map;
};

static ESP8266WebServer server(GDS_WEB_SERVER_PORT);
static HuRequestSender requestSender = nullptr;
static HuWebState webState = {0, 0, 24, 0, 0, 0, 0, 0, 10, 0};
static unsigned long wifiConnectStartMs = 0;
static uint8_t wifiConnecting = 0;

static const char PAGE_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="ko">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>HVAC HU</title>
<style>
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,-apple-system,Segoe UI,sans-serif;background:#f3f5f7;color:#14171a}
header{position:sticky;top:0;background:#ffffff;border-bottom:1px solid #d7dde3;padding:14px 18px;z-index:2}
h1{font-size:20px;margin:0}
main{max-width:960px;margin:0 auto;padding:16px;display:grid;gap:14px}
section{background:#ffffff;border:1px solid #d7dde3;border-radius:8px;padding:14px}
h2{font-size:15px;margin:0 0 12px;color:#303840}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px}
button{width:100%;min-height:42px;border:1px solid #b8c2cc;background:#f9fafb;border-radius:6px;font-weight:650;color:#1c252e}
button:active{transform:translateY(1px)}
button.active{background:#1f6feb;color:white;border-color:#1f6feb}
.row{display:grid;grid-template-columns:90px 1fr 48px;align-items:center;gap:10px;margin:8px 0}
input[type=range]{width:100%}
.status{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:8px}
.pill{border:1px solid #d7dde3;border-radius:6px;padding:8px;background:#fbfcfd}
.label{font-size:12px;color:#59636e}
.value{font-size:18px;font-weight:750;margin-top:2px}
.hint{font-size:12px;color:#59636e;margin-top:8px}
select,input[type=password],input[type=text]{width:100%;min-height:40px;border:1px solid #b8c2cc;border-radius:6px;padding:8px;background:#fff}
.netgrid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
@media(max-width:560px){.netgrid{grid-template-columns:1fr}}
</style>
</head>
<body>
<header><h1>HVAC Head Unit</h1></header>
<main>
  <section>
    <h2>Status</h2>
    <div class="status">
      <div class="pill"><div class="label">Screen</div><div class="value" id="s-screen">DATC</div></div>
      <div class="pill"><div class="label">Power</div><div class="value" id="s-power">OFF</div></div>
      <div class="pill"><div class="label">Fan</div><div class="value" id="s-fan">0</div></div>
      <div class="pill"><div class="label">Temp</div><div class="value" id="s-temp">24</div></div>
      <div class="pill"><div class="label">Wind</div><div class="value" id="s-wind">FACE</div></div>
      <div class="pill"><div class="label">Volume</div><div class="value" id="s-volume">10</div></div>
    </div>
    <div class="hint" id="net">AP: HVAC-HU / 192.168.4.1</div>
  </section>

  <section>
    <h2>Wi-Fi</h2>
    <div class="status">
      <div class="pill"><div class="label">AP</div><div class="value" id="w-ap">HVAC-HU</div></div>
      <div class="pill"><div class="label">External Wi-Fi</div><div class="value" id="w-ssid">Not connected</div></div>
      <div class="pill"><div class="label">External IP</div><div class="value" id="w-ip">-</div></div>
      <div class="pill"><div class="label">RSSI</div><div class="value" id="w-rssi">-</div></div>
    </div>
    <div class="netgrid" style="margin-top:10px">
      <select id="ssid"></select>
      <input id="pass" type="password" placeholder="Password">
    </div>
    <div class="grid" style="margin-top:10px">
      <button onclick="scanWifi()">Scan</button>
      <button onclick="connectWifi()">Connect</button>
      <button onclick="disconnectWifi()">Disconnect</button>
    </div>
    <div class="hint">This page is always available from the HVAC-HU Wi-Fi at 192.168.4.1.</div>
  </section>

  <section>
    <h2>Screen</h2>
    <div class="grid">
      <button onclick="writeSignal(8,0)">DATC</button>
      <button onclick="writeSignal(8,1)">INFO</button>
    </div>
  </section>

  <section>
    <h2>HVAC</h2>
    <div class="grid">
      <button onclick="writeSignal(1,0)">Power Off</button>
      <button onclick="writeSignal(1,1)">Power On</button>
      <button onclick="writeSignal(5,1)">A/C On</button>
      <button onclick="writeSignal(5,0)">A/C Off</button>
      <button onclick="writeSignal(6,1)">Auto On</button>
      <button onclick="writeSignal(6,0)">Auto Off</button>
    </div>
    <div class="row"><span>Fan</span><input id="fan" type="range" min="0" max="8" value="0" oninput="fanOut.textContent=this.value" onchange="writeSignal(2,this.value)"><strong id="fanOut">0</strong></div>
    <div class="row"><span>Temp</span><input id="temp" type="range" min="18" max="30" value="24" oninput="tempOut.textContent=this.value" onchange="writeSignal(3,this.value)"><strong id="tempOut">24</strong></div>
    <div class="grid">
      <button onclick="writeSignal(4,0)">FACE</button>
      <button onclick="writeSignal(4,1)">FOOT</button>
      <button onclick="writeSignal(4,2)">DEF</button>
      <button onclick="writeSignal(4,3)">MIX</button>
    </div>
  </section>

  <section>
    <h2>INFO</h2>
    <div class="grid">
      <button onclick="writeSignal(9,1)">Media Ready</button>
      <button onclick="writeSignal(9,0)">Media Dev</button>
      <button onclick="writeSignal(11,1)">Map Ready</button>
      <button onclick="writeSignal(11,0)">Map Dev</button>
    </div>
    <div class="row"><span>Volume</span><input id="volume" type="range" min="0" max="30" value="10" oninput="volumeOut.textContent=this.value" onchange="writeSignal(10,this.value)"><strong id="volumeOut">10</strong></div>
  </section>
</main>
<script>
const names={screen:["DATC","INFO"],power:["OFF","ON"],wind:["FACE","FOOT","DEF","MIX"],ready:["DEV","READY"]};
function readableWifiName(value, mode){
  if(mode==="connecting") return "Connecting...";
  if(!value) return "Not connected";
  if(value[0]===":" || /^[0-9a-f]{24,}$/i.test(value)) return "Not connected";
  return value;
}
async function writeSignal(signal,value){
  await fetch(`/api/write?signal=${signal}&value=${value}`);
  setTimeout(loadState,120);
}
async function loadState(){
  const r=await fetch("/api/state");
  const s=await r.json();
  document.getElementById("s-screen").textContent=names.screen[s.screen]||s.screen;
  document.getElementById("s-power").textContent=names.power[s.power]||s.power;
  document.getElementById("s-fan").textContent=s.fan;
  document.getElementById("s-temp").textContent=s.temp;
  document.getElementById("s-wind").textContent=names.wind[s.wind]||s.wind;
  document.getElementById("s-volume").textContent=s.volume;
  fan.value=s.fan; fanOut.textContent=s.fan;
  temp.value=s.temp; tempOut.textContent=s.temp;
  volume.value=s.volume; volumeOut.textContent=s.volume;
}
async function loadWifi(){
  const r=await fetch("/api/wifi");
  const w=await r.json();
  document.getElementById("w-ap").textContent=w.apSsid;
  document.getElementById("w-ssid").textContent=readableWifiName(w.ssid,w.mode);
  document.getElementById("w-ip").textContent=w.ip||"-";
  document.getElementById("w-rssi").textContent=w.mode==="connected" ? `${w.rssi} dBm` : "-";
}
async function scanWifi(){
  const r=await fetch("/api/wifi/scan");
  const list=await r.json();
  ssid.innerHTML="";
  list.networks.forEach(n=>{
    const o=document.createElement("option");
    o.value=n.ssid;
    o.textContent=`${n.ssid || "(hidden network)"} (${n.rssi} dBm)`;
    ssid.appendChild(o);
  });
}
async function connectWifi(){
  const body=`ssid=${encodeURIComponent(ssid.value)}&password=${encodeURIComponent(pass.value)}`;
  await fetch("/api/wifi/connect",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body});
  setTimeout(loadWifi,1500);
}
async function disconnectWifi(){
  await fetch("/api/wifi/disconnect",{method:"POST"});
  setTimeout(loadWifi,300);
}
loadState();
loadWifi();
scanWifi();
setInterval(loadState,1000);
setInterval(loadWifi,3000);
</script>
</body>
</html>
)HTML";

static void writeFixedString(uint16_t offset, uint8_t maxLen, const String& value) {
  for (uint8_t i = 0; i < maxLen; i++) {
    char ch = i < value.length() ? value.charAt(i) : '\0';
    EEPROM.write(offset + i, ch);
  }
}

static String readFixedString(uint16_t offset, uint8_t maxLen) {
  char buffer[65];
  uint8_t limit = maxLen < sizeof(buffer) ? maxLen : sizeof(buffer) - 1;

  for (uint8_t i = 0; i < limit; i++) {
    char ch = (char)EEPROM.read(offset + i);
    if ((uint8_t)ch == 0xFF) {
      buffer[i] = '\0';
      return String(buffer);
    }

    buffer[i] = ch;
    if (ch == '\0') {
      buffer[i] = '\0';
      return String(buffer);
    }
  }

  buffer[limit] = '\0';
  return String(buffer);
}

static void saveWifiCredentials(const String& ssid, const String& password) {
  writeFixedString(0, GDS_WIFI_SSID_MAX_LEN, ssid);
  writeFixedString(GDS_WIFI_SSID_MAX_LEN, GDS_WIFI_PASSWORD_MAX_LEN, password);
  EEPROM.commit();
}

static void loadWifiCredentials(String& ssid, String& password) {
  ssid = readFixedString(0, GDS_WIFI_SSID_MAX_LEN);
  password = readFixedString(GDS_WIFI_SSID_MAX_LEN, GDS_WIFI_PASSWORD_MAX_LEN);
}

static void beginStationConnect(const String& ssid, const String& password, uint8_t saveCredentials) {
  if (ssid.length() == 0) {
    return;
  }

  if (saveCredentials) {
    saveWifiCredentials(ssid, password);
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  wifiConnectStartMs = millis();
  wifiConnecting = 1;

  Serial.print("WIFI STA CONNECTING:");
  Serial.println(ssid);
}

static String ipToString(const IPAddress& ip) {
  if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
    return "";
  }

  return ip.toString();
}

static String jsonEscape(const String& value) {
  String escaped = "";

  for (uint16_t i = 0; i < value.length(); i++) {
    char ch = value.charAt(i);
    if (ch == '"' || ch == '\\') {
      escaped += '\\';
    }
    escaped += ch;
  }

  return escaped;
}

static uint8_t isReadableSsid(const String& ssid) {
  if (ssid.length() == 0 || ssid.length() >= GDS_WIFI_SSID_MAX_LEN) {
    return 0;
  }

  if (ssid.charAt(0) == ':') {
    return 0;
  }

  for (uint16_t i = 0; i < ssid.length(); i++) {
    char ch = ssid.charAt(i);
    if (ch < 32 || ch > 126) {
      return 0;
    }
  }

  return 1;
}

static uint8_t parseByteArg(const String& name, uint8_t& value) {
  if (!server.hasArg(name)) {
    return 0;
  }

  String raw = server.arg(name);
  for (uint16_t i = 0; i < raw.length(); i++) {
    if (!isDigit(raw.charAt(i))) {
      return 0;
    }
  }

  int parsed = raw.toInt();
  if (parsed < 0 || parsed > 255) {
    return 0;
  }

  value = (uint8_t)parsed;
  return 1;
}

static void applySignalValue(uint8_t signal, uint8_t value) {
  switch (signal) {
    case CAN_SIGNAL_POWER: webState.power = value; break;
    case CAN_SIGNAL_FAN_SPEED: webState.fan = value; break;
    case CAN_SIGNAL_TEMPERATURE: webState.temp = value; break;
    case CAN_SIGNAL_MODE: webState.wind = value; break;
    case CAN_SIGNAL_AC: webState.ac = value; break;
    case CAN_SIGNAL_AUTO: webState.autoMode = value; break;
    case CAN_SIGNAL_SCREEN_MODE: webState.screen = value; break;
    case CAN_SIGNAL_MEDIA: webState.media = value; break;
    case CAN_SIGNAL_VOLUME: webState.volume = value; break;
    case CAN_SIGNAL_MAP: webState.map = value; break;
  }
}

static void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", PAGE_HTML);
}

static void handleState() {
  String json = "{";
  json += "\"canReady\":";
  json += String(canDriverIsReady());
  json += ",\"power\":";
  json += String(webState.power);
  json += ",\"fan\":";
  json += String(webState.fan);
  json += ",\"temp\":";
  json += String(webState.temp);
  json += ",\"wind\":";
  json += String(webState.wind);
  json += ",\"ac\":";
  json += String(webState.ac);
  json += ",\"auto\":";
  json += String(webState.autoMode);
  json += ",\"screen\":";
  json += String(webState.screen);
  json += ",\"media\":";
  json += String(webState.media);
  json += ",\"volume\":";
  json += String(webState.volume);
  json += ",\"map\":";
  json += String(webState.map);
  json += "}";
  server.send(200, "application/json", json);
}

static void handleWifiStatus() {
  String json = "{";
  json += "\"apSsid\":\"";
  json += jsonEscape(String(GDS_WIFI_AP_SSID));
  json += "\",\"apIp\":\"";
  json += WiFi.softAPIP().toString();
  json += "\",\"mode\":\"";
  json += WiFi.status() == WL_CONNECTED ? "connected" : (wifiConnecting ? "connecting" : "disconnected");
  json += "\",\"ssid\":\"";
  if (WiFi.status() == WL_CONNECTED) {
    json += jsonEscape(WiFi.SSID());
  }
  json += "\",\"ip\":\"";
  json += WiFi.status() == WL_CONNECTED ? ipToString(WiFi.localIP()) : "";
  json += "\",\"rssi\":";
  json += WiFi.status() == WL_CONNECTED ? String(WiFi.RSSI()) : String(0);
  json += "}";
  server.send(200, "application/json", json);
}

static void handleWifiScan() {
  int count = WiFi.scanNetworks();
  String json = "{\"networks\":[";

  for (int i = 0; i < count; i++) {
    String scannedSsid = WiFi.SSID(i);
    if (!isReadableSsid(scannedSsid)) {
      continue;
    }

    if (json.length() > 13) {
      json += ",";
    }

    json += "{\"ssid\":\"";
    json += jsonEscape(scannedSsid);
    json += "\",\"rssi\":";
    json += String(WiFi.RSSI(i));
    json += ",\"secure\":";
    json += WiFi.encryptionType(i) == ENC_TYPE_NONE ? "0" : "1";
    json += "}";
  }

  json += "]}";
  server.send(200, "application/json", json);
}

static void handleWifiConnect() {
  if (!server.hasArg("ssid")) {
    server.send(400, "application/json", "{\"ok\":0,\"error\":\"ssid_required\"}");
    return;
  }

  String ssid = server.arg("ssid");
  String password = server.hasArg("password") ? server.arg("password") : "";
  ssid.trim();

  if (!isReadableSsid(ssid) || password.length() >= GDS_WIFI_PASSWORD_MAX_LEN) {
    server.send(400, "application/json", "{\"ok\":0,\"error\":\"bad_length\"}");
    return;
  }

  beginStationConnect(ssid, password, 1);
  server.send(200, "application/json", "{\"ok\":1}");
}

static void handleWifiDisconnect() {
  saveWifiCredentials("", "");
  WiFi.disconnect();
  wifiConnecting = 0;
  server.send(200, "application/json", "{\"ok\":1}");
}

static void handleWrite() {
  uint8_t signal = 0;
  uint8_t value = 0;

  if (!parseByteArg("signal", signal) || !parseByteArg("value", value)) {
    server.send(400, "application/json", "{\"ok\":0,\"error\":\"bad_arg\"}");
    return;
  }

  if (requestSender == nullptr || !requestSender(CAN_SERVICE_WRITE_REQUEST, signal, value)) {
    server.send(500, "application/json", "{\"ok\":0,\"error\":\"send_fail\"}");
    return;
  }

  applySignalValue(signal, value);
  server.send(200, "application/json", "{\"ok\":1}");
}

static void handleRead() {
  uint8_t signal = 0;
  if (!parseByteArg("signal", signal)) {
    server.send(400, "application/json", "{\"ok\":0,\"error\":\"bad_arg\"}");
    return;
  }

  if (requestSender == nullptr || !requestSender(CAN_SERVICE_READ_REQUEST, signal, 0)) {
    server.send(500, "application/json", "{\"ok\":0,\"error\":\"send_fail\"}");
    return;
  }

  server.send(200, "application/json", "{\"ok\":1}");
}

void webUiBegin(HuRequestSender sender) {
  requestSender = sender;

  EEPROM.begin(GDS_WIFI_EEPROM_SIZE);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(GDS_WIFI_AP_SSID, GDS_WIFI_AP_PASSWORD);

  String savedSsid;
  String savedPassword;
  loadWifiCredentials(savedSsid, savedPassword);
  if (isReadableSsid(savedSsid)) {
    beginStationConnect(savedSsid, savedPassword, 0);
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/write", HTTP_GET, handleWrite);
  server.on("/api/read", HTTP_GET, handleRead);
  server.on("/api/wifi", HTTP_GET, handleWifiStatus);
  server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  server.on("/api/wifi/connect", HTTP_POST, handleWifiConnect);
  server.on("/api/wifi/disconnect", HTTP_POST, handleWifiDisconnect);
  server.begin();

  Serial.print("WEB AP SSID:");
  Serial.println(GDS_WIFI_AP_SSID);
  Serial.print("WEB URL:http://");
  Serial.println(WiFi.softAPIP());
}

void webUiHandle() {
  if (wifiConnecting && WiFi.status() == WL_CONNECTED) {
    wifiConnecting = 0;
    Serial.print("WIFI STA CONNECTED:");
    Serial.println(WiFi.SSID());
    Serial.print("WIFI STA IP:");
    Serial.println(WiFi.localIP());
  }

  if (wifiConnecting && millis() - wifiConnectStartMs > 15000) {
    wifiConnecting = 0;
    Serial.println("WIFI STA CONNECT TIMEOUT");
  }

  server.handleClient();
}

void webUiUpdateFromPayload(const CanPayload& payload) {
  if (!canValidateChecksum(payload)) {
    return;
  }

  if (payload.result == CAN_RESULT_FAIL) {
    return;
  }

  applySignalValue(payload.signal, payload.value);
}
