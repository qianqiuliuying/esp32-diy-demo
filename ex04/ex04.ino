#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 400   // 请根据实际测试调整此值

const unsigned long DEBOUNCE_DELAY = 50; // 防抖时间（毫秒）

bool ledState = false;          // LED 自锁状态
bool lastStableState = false;   // 上一次稳定后的触摸状态（用于检测边沿）
bool lastReading = false;       // 上一次读取的原始状态（用于判断变化）
unsigned long lastChangeTime = 0; // 状态发生变化时的时间戳

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
  Serial.begin(115200);
  delay(100);
}

void loop() {
  // 1. 读取触摸值并转换为布尔状态
  int touchValue = touchRead(TOUCH_PIN);
  bool reading = (touchValue < THRESHOLD);

  // 2. 检测状态变化，记录变化时间
  if (reading != lastReading) {
    lastChangeTime = millis();
  }
  lastReading = reading;

  // 3. 防抖：等待状态稳定（超过 DEBOUNCE_DELAY 时间无变化）
  if ((millis() - lastChangeTime) > DEBOUNCE_DELAY) {
    // 此时状态稳定，当前读取值即为稳定值
    bool stableState = reading;

    // 4. 检测稳定状态的上升沿（从未触摸 → 触摸）
    if (stableState == true && lastStableState == false) {
      ledState = !ledState;          // 翻转 LED 状态
      digitalWrite(LED_PIN, ledState);
      Serial.print("LED toggled, new state: ");
      Serial.println(ledState);
    }

    // 更新上一次稳定状态
    lastStableState = stableState;
  }

}