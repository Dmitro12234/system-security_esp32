#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ---------- Debug switches ----------
constexpr bool DEBUG_NO_WIFI = false;   

// ---------- Wi-Fi ----------
const char* WIFI_SSID = "Name_Wi-Fi";
const char* WIFI_PASS = "Password Wi-FI";
const char* AP_SSID   = "Name Wi-FI";
const char* AP_PASS   = "Password Wi-FI";

// ---------- Pins ----------
constexpr uint8_t PIR_PIN    = 27;
constexpr uint8_t BUTTON_PIN = 13;  
constexpr uint8_t LED_PIN    = 26;
constexpr uint8_t BUZZER_PIN = 25;

// ---------- Settings ----------
constexpr bool     ACTIVE_BUZZER = true;
constexpr uint16_t BUZZER_FREQ   = 2500;
constexpr uint32_t PIR_WARMUP_MS = 30000;
constexpr uint32_t ALARM_MAX_MS  = 60000;
constexpr uint32_t BEEP_MS       = 250;
constexpr uint32_t DEBOUNCE_MS   = 50;
constexpr uint32_t WIFI_TIMEOUT  = 20000;

// ---------- State ----------
bool systemArmed = false;
bool alarmActive = false;
bool pirPrev     = false;
bool beepOn      = false;
const char* lastEvent = "System started";

uint32_t alarmStart = 0;
uint32_t lastBeep   = 0;

WebServer server(80);

// ---------- Web page ----------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Security System</title>
<style>
*{box-sizing:border-box}
body{margin:0;padding:20px;font-family:Arial,sans-serif;background:#101827;color:#f1f5f9}
.c{max-width:480px;margin:20px auto}
h1{text-align:center;font-size:26px}
.card{background:#1e293b;border-radius:16px;padding:20px;margin:16px 0}
.st{font-size:24px;font-weight:bold;text-align:center;margin:10px 0}
.dot{width:12px;height:12px;border-radius:50%;display:inline-block;margin-right:8px;background:#64748b}
button{width:100%;padding:15px;margin-top:10px;border:0;border-radius:10px;color:#fff;font-size:16px;font-weight:bold;cursor:pointer}
button:active{opacity:.7}
.a{background:#16a34a}.d{background:#475569}.s{background:#dc2626}
p{color:#cbd5e1;line-height:1.7;margin:6px 0}
b{color:#fff}
</style></head><body><div class="c">
<h1>ESP32 Security System</h1>
<div class="card">
  <div class="st"><span class="dot" id="dot"></span><span id="st">Connecting...</span></div>
  <p>Motion sensor: <b id="pir">-</b></p>
  <p>PIR warm-up: <b id="rdy">-</b></p>
  <p>Last event: <b id="ev">-</b></p>
</div>
<div class="card">
  <button class="a" onclick="cmd('arm')">ARM SYSTEM</button>
  <button class="d" onclick="cmd('disarm')">DISARM SYSTEM</button>
  <button class="s" onclick="cmd('stop')">STOP SIREN</button>
  <p id="msg"></p>
</div></div>
<script>
const $=id=>document.getElementById(id);
let busy=false;
async function upd(){
  if(busy)return; busy=true;
  try{
    const s=await (await fetch('/api/status',{cache:'no-store'})).json();
    $('st').textContent=s.alarm?'ALARM!':s.armed?'SYSTEM ARMED':'SYSTEM DISARMED';
    $('dot').style.background=s.alarm?'#ef4444':s.armed?'#22c55e':'#64748b';
    $('pir').textContent=s.motion?'MOTION!':'No motion';
    $('rdy').textContent=s.ready?'Ready':'Warming up';
    $('ev').textContent=s.event;
  }catch(e){$('st').textContent='Connection lost';}
  busy=false;
}
async function cmd(c){
  try{
    const r=await fetch('/api/'+c,{method:'POST'});
    $('msg').textContent=(await r.json()).message;
    upd();
  }catch(e){$('msg').textContent='Error: '+e.message;}
}
upd(); setInterval(upd,2000);
</script></body></html>
)rawliteral";

// ---------- Outputs ----------
inline void buzzer(bool on) {
  if (ACTIVE_BUZZER) digitalWrite(BUZZER_PIN, on);
  else if (on) tone(BUZZER_PIN, BUZZER_FREQ);
  else noTone(BUZZER_PIN);
}

void stopAlarm() {
  alarmActive = false;
  beepOn = false;
  buzzer(false);
  digitalWrite(LED_PIN, systemArmed);
}

void startAlarm() {
  alarmActive = true;
  alarmStart = lastBeep = millis();
  beepOn = true;
  buzzer(true);
  lastEvent = "Motion detected";
  Serial.println("ALARM: motion detected");
}

void updateAlarm() {
  if (!alarmActive) return;
  uint32_t now = millis();

  if (now - alarmStart >= ALARM_MAX_MS) {
    stopAlarm();
    lastEvent = "Alarm timeout, still armed";
    return;
  }
  if (now - lastBeep >= BEEP_MS) {
    lastBeep = now;
    beepOn = !beepOn;
    buzzer(beepOn);
    digitalWrite(LED_PIN, beepOn);
  }
}

// ---------- Actions ----------
void doArm()    { systemArmed = true;  pirPrev = digitalRead(PIR_PIN); digitalWrite(LED_PIN, HIGH); lastEvent = "System armed"; }
void doDisarm() { systemArmed = false; stopAlarm(); lastEvent = "System disarmed"; }

bool pirReady() { return millis() >= PIR_WARMUP_MS; }

// ---------- Web handlers ----------
void sendMsg(int code, const char* msg) {
  char buf[96];
  snprintf(buf, sizeof(buf), "{\"message\":\"%s\"}", msg);
  server.send(code, "application/json", buf);
}

void handleStatus() {
  char buf[160];
  snprintf(buf, sizeof(buf),
    "{\"armed\":%s,\"alarm\":%s,\"motion\":%s,\"ready\":%s,\"event\":\"%s\"}",
    systemArmed ? "true" : "false",
    alarmActive ? "true" : "false",
    digitalRead(PIR_PIN) ? "true" : "false",
    pirReady() ? "true" : "false",
    lastEvent);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", buf);
}

void handleArm()    { doArm();    sendMsg(200, pirReady() ? "System armed." : "System armed. Sensor is still warming up."); }
void handleDisarm() { doDisarm(); sendMsg(200, "System disarmed."); }
void handleStop()   { doDisarm(); sendMsg(200, "Siren stopped."); }

// ---------- Wi-Fi ----------
void startWiFi() {
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // low TX power = low current peaks
  WiFi.setSleep(true);                  // modem sleep = lower average current
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Wi-Fi");
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < WIFI_TIMEOUT) {
    delay(250);
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nOpen http://");
    Serial.println(WiFi.localIP());
    if (MDNS.begin("alarm")) Serial.println("or http://alarm.local");
    return;
  }

  Serial.println("\nNo Wi-Fi, starting access point");
  WiFi.disconnect(true);
  delay(200);
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(AP_SSID, AP_PASS, 1, 0, 2);   // channel 1, max 2 clients
  Serial.print("Open http://");
  Serial.println(WiFi.softAPIP());
}

// ---------- Button ----------
void handleButton() {
  static bool lastRead = HIGH, stable = HIGH;
  static uint32_t lastChange = 0;

  bool r = digitalRead(BUTTON_PIN);
  if (r != lastRead) { lastChange = millis(); lastRead = r; }

  if (millis() - lastChange >= DEBOUNCE_MS && r != stable) {
    stable = r;
    if (stable == LOW) {
      if (alarmActive || systemArmed) doDisarm();
      else doArm();
    }
  }
}

// ---------- Setup / Loop ----------
void setup() {
  setCpuFrequencyMhz(80);   // lower CPU clock = lower current
  Serial.begin(115200);
  delay(500);               // let the supply stabilize

  pinMode(BUZZER_PIN, OUTPUT);
  buzzer(false);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!DEBUG_NO_WIFI) {
    startWiFi();
    server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", INDEX_HTML); });
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/arm",    HTTP_ANY, handleArm);
    server.on("/api/disarm", HTTP_ANY, handleDisarm);
    server.on("/api/stop",   HTTP_ANY, handleStop);
    server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
    server.begin();
  } else {
    Serial.println("DEBUG: Wi-Fi disabled");
  }
}

void loop() {
  if (!DEBUG_NO_WIFI) server.handleClient();
  handleButton();

  bool pir = digitalRead(PIR_PIN);
  if (pir && !pirPrev && systemArmed && !alarmActive && pirReady()) {
    startAlarm();
  }
  pirPrev = pir;

  updateAlarm();
  delay(5);
}