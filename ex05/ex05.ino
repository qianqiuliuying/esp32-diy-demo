#define TOUCH_PIN 4          
#define LED_PIN   2         
#define threshold 400   

const unsigned long DEBOUNCE_DELAY = 50;   // 防抖时间(ms)

bool lastReading = false;          // 上一次原始读取值
bool lastStableState = false;      // 上一次稳定后的状态
unsigned long lastChangeTime = 0;  // 状态变化时间戳

int brightness = 0;            // 当前亮度 (0~255)
int direction = 1;             // 方向：1递增，-1递减
int stepSize = 1;              // 当前步长（由档位决定）
int speedLevel = 1;            // 当前档位 (1,2,3)
const unsigned long UPDATE_INTERVAL = 10;  // 呼吸更新间隔(ms)
unsigned long lastUpdateTime = 0;

const int freq = 5000;         // 频率 5000Hz
const int resolution = 8;      // 分辨率 8位 (0-255)

void setup() {
  Serial.begin(115200);
  delay(1000); // 等待串口稳定

  ledcAttach(LED_PIN, freq, resolution); 
  ledcWrite(LED_PIN, 0);                 

}

void loop() {
  int touchValue = touchRead(TOUCH_PIN);
  bool reading = (touchValue < threshold);  // true表示触摸

  // 检测状态变化（边缘触发）
  if (reading != lastReading) {
    lastChangeTime = millis();   // 记录变化时刻
  }
  lastReading = reading;  // 更新上一次原始状态

  // 软件防抖
  if ((millis() - lastChangeTime) > DEBOUNCE_DELAY) {
    bool stableState = reading;

    // 上升沿检测：从未触摸 -> 触摸（即按下的瞬间）
    if (stableState == true && lastStableState == false) {
      
      speedLevel++;
      if (speedLevel > 3) speedLevel = 1;

      // 根据档位设定步长（步长越大呼吸越快）
      switch (speedLevel) {
        case 1: stepSize = 1; break;
        case 2: stepSize = 3; break;
        case 3: stepSize = 5; break;
      }
      
      // 串口输出当前档位
      Serial.print("Speed Level: ");
      Serial.println(speedLevel);
    }
    lastStableState = stableState;  // 更新稳定状态
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateTime >= UPDATE_INTERVAL) 
  {
    lastUpdateTime = currentMillis;

    // 亮度步进（使用当前档位决定的步长）
    brightness += stepSize * direction;

    // 边界反转（到达最亮或最暗时改变方向）
    if (brightness >= 255) 
    {
      brightness = 255;
      direction = -1;
    } else if (brightness <= 0) 
    {
      brightness = 0;
      direction = 1;
    }

    ledcWrite(LED_PIN, brightness);
  }
}