//ESP8266
#include <SoftwareSerial.h>
SoftwareSerial sSerial(4,5);
String ssid ="ca87";  //WIFI ID
String pwd ="02160216"; //WIFI 密碼
String GET = "GET /ADO/public/create"; //傳送資料位置
String serverPlace = "caphp.ddns.net"; 
String cd;//資料存放
/*TX         GND
 *CH_PD→|   GPIO2
 *RST    |   GPIO0
 *3.3V →|   RX
 */
//MQ2 VCC 5V 
#include <MQ2.h>
int pin = A0;         //MQ2 A0
int lpg, co, smoke;  //液化石油氣 一氧化碳 煙霧 數值
int mq7co; //MQ7一氧化碳 數值
int MQ2LPG,MQ2smoke,MQ7co; //多餘的數值設定
MQ2 mq2(pin); //
//temp VCC 5V
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2                  // 溫度在2腳位
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//MQ7 VCC 5V
const int AOUTpin=A1;//the AOUT pin of the CO sensor goes into analog pin A0 of the arduino  MQ7 A1

int value;

//fan
const int fan=9;
int fanset =0;

void setup() {
  Serial.begin(9600);
  sSerial.begin(9600);              //啟始軟體串列埠 (與 ESP8266 介接用)
  esp8266set();  //呼叫ESP8266設定
  Serial.println("Start"); //開始偵測
  mq2.begin();  
  pinMode(fan, OUTPUT);//sets the pin as an output of the arduino
  pinMode(A2, INPUT);//sound
  sensors.begin();
}

void loop() {
   fans(); //風扇控制
   sound(); //聲音感測
   mq_2(); //MQ2感測
   mq7(); //MQ7感測
   temp(); //溫度感測
   MQ2LPG= mq2.readLPG(); //抓取LPG數值
   MQ2smoke = mq2.readSmoke();//抓取煙霧數值
   MQ7co= analogRead(AOUTpin);//reads the analaog value from the CO sensor's AOUT pin
   if (fanset == 1) {//當風扇控制等於1優先度大於下列
   }
   if(MQ7co>200) {//當丙烷 >13200 開啟風扇
    digitalWrite(fan,HIGH);
   }
   if(MQ2LPG>13200){//當丙烷 >13200 開啟風扇
     digitalWrite(fan,HIGH);
   }
   if (MQ2smoke>0) {//當有煙霧 開啟風扇
    digitalWrite(fan,HIGH);
   }
   if (fanset !=1 && MQ7co < 200 && MQ2LPG < 13200 && MQ2smoke == 0 ) {
    digitalWrite(fan,LOW);  //都沒有 關閉風扇
   }   
   DataGet(cd);
  delay(200);
}
//ESP8266設定
void esp8266set(){
   Serial.println("AT+RST");
  sSerial.println("AT+RST");        //軟體串列埠傳送 AT 指令重啟 ESP8266
  delay(5000);
  sSerial.println("AT+CWMODE=3");
  Serial.println("AT+CWMODE=3");
  delay(5000);
  sSerial.println("AT+CWJAP=\"" + ssid + "\",\"" + pwd + "\"");   // esp8266 WiFi帳密
  Serial.println("AT+CWJAP=\"" + ssid + "\",\"" + pwd + "\"");
  delay(5000);
}
//風扇控制
void fans(){
  //連接伺服器
  String cmd="AT+CIPSTART=\"TCP\",\"" + serverPlace + "\",8080";
  sSerial.println(cmd);
  if (sSerial.find("Error")) {
    return;
  }
  Serial.println(cmd);
  //告訴ESP8266要傳送的字串長度
  String fanControl = "GET /esptext.php\r\n\r\n";
  String cmds = "AT+CIPSEND=" + String(fanControl.length());
  Serial.println(cmds);
  sSerial.println(cmds);  
  //找到>
  if (sSerial.find(">")) {
    //連接esptext.php
    sSerial.print(fanControl);
    Serial.print(">" + fanControl);
  }
  else {
  sSerial.println("AT+CIPCLOSE");
  Serial.println("connect timeout");
  delay(1000);
  return;
  }
  //讀取網址資料
  while (sSerial.available()) {
    if (sSerial.find("+IPD,")) {
      delay(1000); 
      int connectionId=sSerial.read()-48;  //turn ASCII to number
        while (sSerial.findUntil("pin", "\r\n")) {
          char type=(char)sSerial.read();
          int pin=sSerial.parseInt();
          int val=sSerial.parseInt();
          if (type=='D') {
            fanset = 1;
            Serial.print("Digital pin ");
            pinMode(pin, OUTPUT);
            digitalWrite(pin, val);        
            }
          else if (type=='A') {
            Serial.print("Analog pin ");
            analogWrite(pin, val);            
            }
          else {
            Serial.println("Unexpected type:" + type);
            }
          Serial.print(pin);
          Serial.print("=");
          Serial.println(val);         
          }
      }
  }
  /*delay(2000);
  while (sSerial.available()) {
    char c = sSerial.read();
    Serial.write(c);
    if (c == '\r') {
      Serial.print('\n');
    }    
  }
  Serial.println("====");
  delay(1000);*/
}

//聲音感測
void sound(){
  value = analogRead(A2);
  Serial.print("聲音:");
  Serial.println(value);
  cd="&sound="+String(value);
  }

//MQ2感測
void mq_2(){
  float* values= mq2.read(true); //set it false if you don't want to print the values in the Serial
   
  //lpg = values[0];
  lpg = mq2.readLPG();
  cd+="&mq2_lpg="+String(lpg);
  //co = values[1];
  co = mq2.readCO();
  cd+="&mq2_co="+String(co);
  //smoke = values[2];
  smoke = mq2.readSmoke();
  cd+="&mq2_smoke="+String(smoke);
}
//MQ7感測
void mq7(){
  mq7co= analogRead(AOUTpin);//reads the analaog value from the CO sensor's AOUT pin
  Serial.print("CO value: ");
  Serial.print(mq7co);//prints the CO value
  cd+="&mq7_co="+String(mq7co);
}
//溫度感測
void temp(){
  sensors.requestTemperatures();
  Serial.print("  溫度: ");
  
  int temp=int(sensors.getTempCByIndex(0));
  Serial.print(temp);
  Serial.println("度");
  cd+="&temp="+String(temp);
  delay(1000);
}

//資料傳送
void DataGet(String c){
  //連接伺服器位置
  String cmd="AT+CIPSTART=\"TCP\",\"" + serverPlace + "\",8080";
  sSerial.println(cmd);  
  if (sSerial.find("Error")) {
  Serial.println("AT+CIPSTART error!");
  return;  //連線失敗跳出目前迴圈 (不做後續傳送作業)
  }
  Serial.println(cmd);
  //創輸出數值字串  
  String data = GET + c +"\r\n\r\n"; 
  Serial.println(data);  //顯示 data 字串內容於監控視窗 
  //告知 ESP8266 即將傳送之 GET 字串長度
  String cmds="AT+CIPSEND=" + String(data.length()); //傳送 GET 字串長度之 AT 指令
  sSerial.println(cmds); 
  Serial.println(cmds);
  //檢查 ESP8266 是否回應
  if (sSerial.find(">")) {  //若收到 ESP8266 的回應標頭結束字元
    sSerial.print(data);  //向 ESP8266 傳送 GET 字串內容   
    }
  else {  //沒有收到 ESP8266 回應
    sSerial.println("AT+CIPCLOSE");  //關閉 TCP 連線
    Serial.println("AT+CIPCLOSE");
  }
  delay(5000); 
} 
