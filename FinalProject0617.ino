#include <LWiFi.h>
#include "DHT.h"
#include "MCS.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27);
char _lwifi_ssid[] = "琦妹";
char _lwifi_pass[] = "love0420";
int afterButton1 = 0;
int afterButton2 = 0;
int state1 = 0;
int state2 = 0;
int buttonState1 = 0;
int buttonState2 = 0;
int buzzerstate =0;
const double alpha = 0.75;
const double beta = 0.5;
const int period = 20;
float t;
int count = 0;
int tones[] = {1000,1500,5000};
MCSDevice mcs("DNeP1Jbg", "CdPu4r2zM8Z02ysK");
MCSDisplayInteger temp("temp");
MCSDisplayInteger heart("heart");
MCSControllerOnOff MySwitch("alter_led");
//LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT __dht2(2, DHT22);

void setup()
{
  Serial.begin(9600);
  pinMode(17, INPUT);
  pinMode(2, INPUT);
  pinMode(5, INPUT);
  pinMode(3, OUTPUT);
  mcs.addChannel(temp);
  mcs.addChannel(heart);
  //  連接MCS
  mcs.addChannel(MySwitch);
  Serial.println("Wi-Fi 開始連線");
  while (WiFi.begin(_lwifi_ssid, _lwifi_pass) != WL_CONNECTED) {
    delay(1000);

  }
  Serial.println("Wi-Fi 連線成功");
  while (!mcs.connected()) {
    mcs.connect();
  }
  Serial.println("MCS 連線成功");
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  lcd.begin(16, 2);
 

  
  __dht2.begin();
}

void loop(){
if(buzzerstate == 1){
  tone(3,392, 1000);
  }else if (state2 == 1 || buzzerstate == 0){
    noTone(3);
    }
  buttonState1 = digitalRead(2);
  buttonState2 = digitalRead(5);
  if (buttonState2 ==LOW&& afterButton2 == HIGH  ) {
    state2 = 1 - state2;
    delay(10);
  }
  if (buttonState1 ==  LOW && afterButton1 ==HIGH) {
    state1 = 1 - state1;
    delay(10);
  }
  afterButton1 = buttonState1;
  afterButton2 = buttonState2;

  if (state2 == 1) {
    Serial.println("RB 1");
    lcd.clear();
    digitalWrite(LED_BUILTIN,LOW);
    buzzerstate = 0;
  } else  if (state2 == 0) {
    Serial.println("RB 0");
//tone(3,392,300);
  }

  if (state1 == 1) {
    Serial.println("YB READ!!!!!");
    senseHeartRate();

    t = __dht2.readTemperature();
    Serial.print("體溫：");
    Serial.println(t);
    Serial.print("心律：");
    Serial.println(count);
    lcdWrite();
    temp.set(t);
    count = __dht2.readHumidity();
  } else if (state1 == 0) {
    Serial.println("YB STOP!!!!!");
    delay(1000);
  }

  while (!mcs.connected()) {
    mcs.connect();
    if (mcs.connected()) {
      Serial.println("MCS 已重新連線");
    }
  }
  mcs.process(100);

  if (MySwitch.updated()) {
    Serial.print("控制通道更新 :");
    Serial.println(MySwitch.value());

    if (MySwitch.value()) {
      digitalWrite(LED_BUILTIN, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Take medicine");
      buzzerstate =1;
    }
    state2 = 0;
    delay(1000);
  }
}
//心跳感測
void senseHeartRate()
{
  double oldValue = 0;                        // 記錄上一次sense到的值
  double oldChange = 0;                       // 記錄上一次值的改變

  unsigned long startTime = millis();         // 記錄開始測量時間

  while (millis() - startTime < 10000) {      // sense 10 seconds
    int rawValue = analogRead(17);    // 讀取心跳sensor的值
    double value = alpha * oldValue + (1 - alpha) * rawValue; //smoothing value

    //find peak
    double change = value - oldValue;       // 計算跟上一次值的改變量
    if (change > beta && oldChange < -beta) { // heart beat
      count = count + 1;
    }

    oldValue = value;
    oldChange = change;
    delay(period);
  }
  heart.set(count);

  //BTSerial.println(count*6);          //use bluetooth to send result to android
}
void lcdWrite() {
  //心跳
  if (count > 100) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Heart:");
    lcd.print(count);
  }
  //體溫
  if (t > 38 || t < 35) {
    lcd.setCursor(0, 1);
    lcd.print("Temperature:");
    lcd.print(t);
    lcd.println(" *C");
  }
  delay(500);
}
