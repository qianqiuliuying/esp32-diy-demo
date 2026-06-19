const int ledPin = 2;

unsigned long currentTime;
unsigned long previousTime = 0;

const unsigned long Time1 = 500; // 短点亮时长 (ms)   
const unsigned long Time2 = 1500;   // 长点亮时长 (ms)
const unsigned long GAP_OFF = 500;  // 熄灭间隔 (ms)
const unsigned long END_WAIT = 2000;// 整套SOS结束停顿 (ms)
int stage = 0;      // 0:三短闪, 1:三长闪, 2:三短闪, 3:停顿
int count = 0;      // 当前阶段已完成的闪烁次数（每次熄灭后+1）
bool state = false; // true=灯亮, false=灯灭

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);   // 初始熄灭
  state = false;
  previousTime = millis();     // 开始计时
}

void loop() {
  currentTime = millis();
  unsigned long gap = currentTime - previousTime;

  switch (stage) 
  {
    case 0: // 三短闪 ···
      if (state) 
      { // 灯亮，等待短闪时长后熄灭
        if (gap >= Time1) 
        {
          digitalWrite(ledPin, LOW);
          state = false;
          previousTime = currentTime;
          count++;                // 完成一次短闪
          if (count >= 3) 
          {       // 三次完成，进入长闪阶段
            stage = 1;
            count = 0;
          }
        }
      } 
      else 
      { // 灯灭，等待间隔后点亮
        if (gap >= GAP_OFF) 
        {
          digitalWrite(ledPin, HIGH);
          state = true;
          previousTime = currentTime;
        }
      }
      break;

    case 1: // 三长闪 ———
      if (state) 
      {
        if (gap >= Time2) 
        {
          digitalWrite(ledPin, LOW);
          state = false;
          previousTime = currentTime;
          count++;
          if (count >= 3) 
          {
            stage = 2;
            count = 0;
          }
        }
      } else 
      {
        if (gap >= GAP_OFF) 
        {
          digitalWrite(ledPin, HIGH);
          state = true;
          previousTime = currentTime;
        }
      }
      break;

    case 2: // 再三次短闪 ···
      if (state) 
      {
        if (gap >= Time1) 
        {
          digitalWrite(ledPin, LOW);
          state = false;
          previousTime = currentTime;
          count++;
          if (count >= 3) 
          {       // 三次完成，进入结束停顿
            stage = 3;
            count = 0;
          }
        }
      } else 
      {
        if (gap >= GAP_OFF) 
        {
          digitalWrite(ledPin, HIGH);
          state = true;
          previousTime = currentTime;
        }
      }
      break;

    case 3: // 结束停顿（灯灭）
      if (gap >= END_WAIT) 
      {
        stage = 0;               // 从头开始
        count = 0;
        state = false;           // 确保灯灭
        previousTime = currentTime;
      }
      break;
  }
}