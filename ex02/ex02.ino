// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2; 

unsigned long currentTime;
unsigned long previousTime=0;
const unsigned long Time=500;//定时间隔1s，1Hz

void setup() {
  // 初始化串口通信，设置波特率为115200
  Serial.begin(115200);
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
}

void loop() {
  currentTime=millis();// 获取当前总运行毫秒
    
  if (currentTime-previousTime>=Time)// 判断是否达到间隔时间
  {
   previousTime=currentTime;// 更新记录时间

   digitalWrite(ledPin, !digitalRead(ledPin));
   if(digitalRead(ledPin) == HIGH)
   {
      Serial.println("LED ON");// 小灯亮串口输出提示
   }
   else
   {
      Serial.println("LED OFF");// 小灯灭串口输出提示
   }
    
  }

}