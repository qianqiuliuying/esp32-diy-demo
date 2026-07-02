#include <WiFi.h>
#include <WebServer.h>

// ===== AP 热点配置 =====
const char* ap_ssid = "ESP32-LAB008";
const char* ap_password = "12345678";   // 至少8位

#define TOUCH_PIN 4
#define LED_PIN   2

int threshold = 400;

const unsigned long DEBOUNCE_DELAY = 50;

// ===== 系统状态 =====
bool armed = false;
bool alarmTriggered = false;

// ===== 触摸防抖变量 =====
bool lastReading = false;
bool lastStableState = false;
unsigned long lastChangeTime = 0;

// ===== LED闪烁控制 =====
const unsigned long BLINK_INTERVAL = 200;
unsigned long lastBlinkTime = 0;
bool ledState = false;

WebServer server(80);

// ===== 构建 HTML 页面（含自动刷新状态） =====
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>物联网安防报警器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; background: #f5f5f5; }
    .container { background: white; padding: 30px; border-radius: 20px; max-width: 400px; margin: auto; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    .status { font-size: 22px; margin: 15px 0; }
    .armed { color: #dc3545; }
    .disarmed { color: #28a745; }
    .alarm { color: #ff0000; font-weight: bold; font-size: 28px; }
    .btn-group { margin-top: 20px; }
    button { padding: 15px 30px; margin: 8px; border: none; border-radius: 10px; font-size: 18px; cursor: pointer; }
    .btn-arm { background: #dc3545; color: white; }
    .btn-disarm { background: #28a745; color: white; }
    .btn-arm:hover { background: #c82333; }
    .btn-disarm:hover { background: #218838; }
    .footer { margin-top: 30px; color: #888; font-size: 14px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔐 安防报警器</h1>
    <div class="status">
      系统状态：<span id="armStatus" class="disarmed">🟢 已撤防</span>
    </div>
    <div class="status">
      报警状态：<span id="alarmStatus" class="alarm" style="color:#28a745;">✅ 正常</span>
    </div>
    <div class="btn-group">
      <button class="btn-arm" onclick="sendCommand('arm')">🔒 布防</button>
      <button class="btn-disarm" onclick="sendCommand('disarm')">🔓 撤防</button>
    </div>
    <div class="footer">触摸引脚 (GPIO4) 触发报警</div>
  </div>

  <script>
    // 发送命令（布防/撤防）
    function sendCommand(cmd) {
      fetch('/' + cmd)
        .then(response => response.text())
        .then(data => {
          console.log('Command executed: ' + cmd);
        })
        .catch(error => {
          alert('网络错误，请重试');
          console.error('Error:', error);
        });
    }

    // 定时获取最新状态（每 500ms）
    function fetchStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          // 更新布防状态
          const armSpan = document.getElementById('armStatus');
          if (data.armed) {
            armSpan.textContent = '🔴 已布防';
            armSpan.className = 'armed';
          } else {
            armSpan.textContent = '🟢 已撤防';
            armSpan.className = 'disarmed';
          }

          // 更新报警状态
          const alarmSpan = document.getElementById('alarmStatus');
          if (data.alarm) {
            alarmSpan.textContent = '🚨 报警中！';
            alarmSpan.style.color = '#ff0000';
          } else {
            alarmSpan.textContent = '✅ 正常';
            alarmSpan.style.color = '#28a745';
          }
        })
        .catch(error => {
          console.error('Fetch status error:', error);
        });
    }

    // 页面加载后立即获取一次，并启动定时器
    window.onload = function() {
      fetchStatus();
      setInterval(fetchStatus, 500); // 每 500ms 更新一次
    };
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// ===== Web 路由处理 =====
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

void handleArm() {
  armed = true;
  alarmTriggered = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "Armed");
  Serial.println("System Armed");
}

void handleDisarm() {
  armed = false;
  alarmTriggered = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "Disarmed");
  Serial.println("System Disarmed");
}

// ===== 状态查询接口（手动拼接 JSON，无库依赖） =====
void handleStatus() {
  String json = "{";
  json += "\"armed\":" + String(armed ? "true" : "false") + ",";
  json += "\"alarm\":" + String(alarmTriggered ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// ===== 触摸检测 =====
void checkTouch() {
  int touchValue = touchRead(TOUCH_PIN);
  bool reading = (touchValue < threshold);

  if (reading != lastReading) {
    lastChangeTime = millis();
  }
  lastReading = reading;

  if ((millis() - lastChangeTime) > DEBOUNCE_DELAY) {
    bool stableState = reading;
    if (stableState == true && lastStableState == false) {
      if (armed && !alarmTriggered) {
        alarmTriggered = true;
        Serial.println("🚨 ALARM TRIGGERED!");
      }
    }
    lastStableState = stableState;
  }
}

// ===== LED 报警闪烁 =====
void handleAlarmBlink() {
  if (alarmTriggered) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    ledState = false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ----- 开启 AP 热点（自发热点） -----
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("==== ESP32 AP 启动成功 ====");
  Serial.print("热点名称："); Serial.println(ap_ssid);
  Serial.print("访问地址："); Serial.println(WiFi.softAPIP());  // 通常 192.168.4.1

  // ----- 路由绑定 -----
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("Web服务器已启动，安防报警器就绪");
}

void loop() {
  server.handleClient();
  checkTouch();
  handleAlarmBlink();
}