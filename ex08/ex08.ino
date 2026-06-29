#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "vivo S30";
const char* password = "excellentlgg";

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


//  构建HTML页面
String makePage() {
  // 状态文字
  String statusText = armed ? "🔴 已布防" : "🟢 已撤防";
  String alarmText = alarmTriggered ? "🚨 报警中！" : "✅ 正常";
  
  // 状态class
  String armClass = armed ? "armed" : "disarmed";

  // 使用 String 拼接构建 HTML
  String html = "<!DOCTYPE html>\n";
  html += "<html lang=\"zh-CN\">\n";
  html += "<head>\n";
  html += "  <meta charset=\"UTF-8\">\n";
  html += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  html += "  <title>物联网安防报警器</title>\n";
  html += "  <style>\n";
  html += "    body { font-family: Arial; text-align: center; margin-top: 50px; background: #f5f5f5; }\n";
  html += "    .container { background: white; padding: 30px; border-radius: 20px; max-width: 400px; margin: auto; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }\n";
  html += "    h1 { color: #333; }\n";
  html += "    .status { font-size: 22px; margin: 15px 0; }\n";
  html += "    .armed { color: #dc3545; }\n";
  html += "    .disarmed { color: #28a745; }\n";
  html += "    .alarm { color: #ff0000; font-weight: bold; font-size: 28px; }\n";
  html += "    .btn-group { margin-top: 20px; }\n";
  html += "    button { padding: 15px 30px; margin: 8px; border: none; border-radius: 10px; font-size: 18px; cursor: pointer; }\n";
  html += "    .btn-arm { background: #dc3545; color: white; }\n";
  html += "    .btn-disarm { background: #28a745; color: white; }\n";
  html += "    .btn-arm:hover { background: #c82333; }\n";
  html += "    .btn-disarm:hover { background: #218838; }\n";
  html += "    .footer { margin-top: 30px; color: #888; font-size: 14px; }\n";
  html += "  </style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <div class=\"container\">\n";
  html += "    <h1>🔐 安防报警器</h1>\n";
  html += "    <div class=\"status\">\n";
  html += "      系统状态：<span id=\"armStatus\" class=\"" + armClass + "\">" + statusText + "</span>\n";
  html += "    </div>\n";
  html += "    <div class=\"status\">\n";
  html += "      报警状态：<span id=\"alarmStatus\" class=\"alarm\">" + alarmText + "</span>\n";
  html += "    </div>\n";
  html += "    <div class=\"btn-group\">\n";
  html += "      <button class=\"btn-arm\" onclick=\"sendCommand('arm')\">🔒 布防</button>\n";
  html += "      <button class=\"btn-disarm\" onclick=\"sendCommand('disarm')\">🔓 撤防</button>\n";
  html += "    </div>\n";
  html += "    <div class=\"footer\">触摸引脚 (GPIO4) 触发报警</div>\n";
  html += "  </div>\n";
  html += "  <script>\n";
  html += "    function sendCommand(cmd) {\n";
  html += "      fetch('/' + cmd)\n";
  html += "        .then(response => response.text())\n";
  html += "        .then(data => { location.reload(); })\n";
  html += "        .catch(error => { alert('网络错误，请重试'); console.error('Error:', error); });\n";
  html += "    }\n";
  html += "  </script>\n";
  html += "</body>\n";
  html += "</html>\n";
  
  return html;
}

//  Web路由处理
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


//  触摸检测
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


//  LED报警闪烁
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

  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();
  Serial.println("Web服务器已启动，安防报警器就绪");
}

void loop() {
  server.handleClient();
  checkTouch();
  handleAlarmBlink();
}