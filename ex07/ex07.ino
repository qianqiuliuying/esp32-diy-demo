#include <WiFi.h>
#include <WebServer.h>

// ===== AP 热点配置 =====
const char* ap_ssid = "ESP32-LAB008";
const char* ap_password = "12345678";   // 至少8位

const int LED_PIN = 2;

const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

int currentBrightness = 0;

WebServer server(80);

// ===== 构建 HTML 页面（含滑动条） =====
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; background: #f0f0f0; }
    .container { background: white; padding: 30px; border-radius: 20px; max-width: 400px; margin: auto; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    .slider { width: 80%; margin: 20px 0; }
    .value { font-size: 24px; font-weight: bold; color: #007BFF; }
    .btn-group { margin-top: 20px; }
    button { padding: 10px 25px; margin: 5px; border: none; border-radius: 8px; font-size: 16px; cursor: pointer; }
    .btn-on { background: #28a745; color: white; }
    .btn-off { background: #dc3545; color: white; }
    .status { margin-top: 20px; font-size: 18px; color: #555; }
  </style>
</head>
<body>
  <div class="container">
    <h1>💡 无极调光器</h1>
    <p>拖动滑块调节亮度</p>
    <input type="range" id="brightnessSlider" class="slider" min="0" max="255" value="0">
    <div>
      <span>亮度值：</span><span id="brightnessValue" class="value">0</span>
    </div>
    <div class="btn-group">
      <button class="btn-on" onclick="setBrightness(255)">点亮</button>
      <button class="btn-off" onclick="setBrightness(0)">熄灭</button>
    </div>
    <div class="status" id="status">状态：待机</div>
  </div>

  <script>
    // 获取滑块和显示元素
    const slider = document.getElementById('brightnessSlider');
    const valueDisplay = document.getElementById('brightnessValue');
    const statusDisplay = document.getElementById('status');

    // 发送亮度值到 ESP32
    function sendBrightness(value) {
      fetch('/set?value=' + value)
        .then(response => {
          if (response.ok) {
            statusDisplay.innerHTML = '✅ 已更新亮度：' + value;
          } else {
            statusDisplay.innerHTML = '❌ 更新失败';
          }
        })
        .catch(error => {
          statusDisplay.innerHTML = '⚠️ 网络错误';
          console.error('Error:', error);
        });
    }

    // 滑块拖动时触发
    slider.oninput = function() {
      const val = parseInt(this.value);
      valueDisplay.textContent = val;
      sendBrightness(val);
    };

    // 快捷按钮（点亮/熄灭）
    function setBrightness(value) {
      slider.value = value;
      valueDisplay.textContent = value;
      sendBrightness(value);
    }
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// ===== 设置亮度 =====
void handleSet() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    currentBrightness = val;
    ledcWrite(LED_PIN, currentBrightness);   // 更新 PWM
    server.send(200, "text/plain", "OK");
    Serial.print("Brightness set to: ");
    Serial.println(currentBrightness);
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter");
  }
}

// 点亮
void handleOn() {
  currentBrightness = 255;
  ledcWrite(LED_PIN, currentBrightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

// 熄灭
void handleOff() {
  currentBrightness = 0;
  ledcWrite(LED_PIN, currentBrightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 初始化 PWM
  ledcAttach(LED_PIN, freq, resolution);
  ledcWrite(LED_PIN, 0);      // 初始熄灭

  // ----- 开启 AP 热点（自发热点） -----
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("==== ESP32 AP 启动成功 ====");
  Serial.print("热点名称："); Serial.println(ap_ssid);
  Serial.print("访问地址："); Serial.println(WiFi.softAPIP());  // 通常 192.168.4.1

  // ----- 配置 Web 路由 -----
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.begin();
  Serial.println("Web服务器已启动");
}

void loop() {
  server.handleClient();
}