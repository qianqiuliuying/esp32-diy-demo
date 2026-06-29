#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "vivo S30";
const char* password = "excellentlgg";

#define TOUCH_PIN 4  

WebServer server(80);


//  构建HTML页面（含仪表盘显示和AJAX轮询）
String makePage() {
  String html = "<!DOCTYPE html>\n";
  html += "<html lang=\"zh-CN\">\n";
  html += "<head>\n";
  html += "  <meta charset=\"UTF-8\">\n";
  html += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  html += "  <title>传感器实时仪表盘</title>\n";
  html += "  <style>\n";
  html += "    body { font-family: 'Segoe UI', Arial; text-align: center; margin-top: 50px; background: #1e1e2f; color: #fff; }\n";
  html += "    .container { background: #2d2d44; padding: 40px; border-radius: 30px; max-width: 500px; margin: auto; box-shadow: 0 8px 20px rgba(0,0,0,0.5); }\n";
  html += "    h1 { font-size: 28px; margin-bottom: 10px; }\n";
  html += "    .value-box { background: #0f0f1a; padding: 30px; border-radius: 20px; margin: 20px 0; border: 2px solid #4a6fa5; }\n";
  html += "    .value { font-size: 72px; font-weight: bold; color: #4fc3f7; font-family: 'Courier New', monospace; }\n";
  html += "    .unit { font-size: 24px; color: #aaa; margin-left: 10px; }\n";
  html += "    .status { font-size: 18px; color: #aaa; margin-top: 10px; }\n";
  html += "    .bar { width: 100%; height: 10px; background: #444; border-radius: 10px; margin-top: 20px; overflow: hidden; }\n";
  html += "    .bar-fill { height: 100%; width: 0%; background: linear-gradient(90deg, #4fc3f7, #0288d1); border-radius: 10px; transition: width 0.2s; }\n";
  html += "    .footer { margin-top: 30px; font-size: 14px; color: #666; }\n";
  html += "  </style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <div class=\"container\">\n";
  html += "    <h1>📊 触摸传感器仪表盘</h1>\n";
  html += "    <div class=\"value-box\">\n";
  html += "      <div class=\"value\" id=\"sensorValue\">--</div>\n";
  html += "      <span class=\"unit\">(原始值)</span>\n";
  html += "    </div>\n";
  html += "    <div class=\"bar\">\n";
  html += "      <div class=\"bar-fill\" id=\"barFill\"></div>\n";
  html += "    </div>\n";
  html += "    <div class=\"status\" id=\"statusText\">等待数据...</div>\n";
  html += "    <div class=\"footer\">手指靠近GPIO4，数值减小</div>\n";
  html += "  </div>\n";
  html += "\n";
  html += "  <script>\n";
  html += "    // 定时获取传感器数据\n";
  html += "    function fetchData() {\n";
  html += "      fetch('/touch')\n";
  html += "        .then(response => response.text())\n";
  html += "        .then(data => {\n";
  html += "          const num = parseInt(data);\n";
  html += "          if (!isNaN(num)) {\n";
  html += "            document.getElementById('sensorValue').textContent = num;\n";
  html += "            // 更新进度条（假设最大值为800，可根据实际调整）\n";
  html += "            const maxVal = 800;\n";
  html += "            const percent = Math.min(100, (num / maxVal) * 100);\n";
  html += "            document.getElementById('barFill').style.width = percent + '%';\n";
  html += "            document.getElementById('statusText').textContent = '🟢 实时更新中';\n";
  html += "          }\n";
  html += "        })\n";
  html += "        .catch(error => {\n";
  html += "          document.getElementById('statusText').textContent = '🔴 连接失败';\n";
  html += "          console.error('Error:', error);\n";
  html += "        });\n";
  html += "    }\n";
  html += "\n";
  html += "    // 每200ms刷新一次\n";
  html += "    setInterval(fetchData, 200);\n";
  html += "    // 立即执行一次\n";
  html += "    fetchData();\n";
  html += "  </script>\n";
  html += "</body>\n";
  html += "</html>\n";
  
  return html;
}

//  Web路由处理
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 返回触摸原始值
void handleTouch() {
  int value = touchRead(TOUCH_PIN);
  server.send(200, "text/plain", String(value));
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 配置路由
  server.on("/", handleRoot);
  server.on("/touch", handleTouch);

  server.begin();
  Serial.println("Web服务器已启动，实时仪表盘就绪");
}

void loop() {
  server.handleClient();
}