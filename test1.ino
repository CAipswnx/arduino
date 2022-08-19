#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
//--------------------------------------------
char SSID[] = "SSID";
char PASSWORD[] = "WIFI密碼";
String Linetoken = "LINE TOKEN";
char host[] = "notify-api.line.me";
int pinDHT11 = 4;//溫溼度
int buzzer = 17;//蜂鳴器
//---------------------------------------------------------
//DHT11
SimpleDHT11 dht11(pinDHT11);
//WiFi
WiFiClientSecure client;

//LCD SDA 21/SCL22
LiquidCrystal_I2C lcd(0x27, 16, 2);

//time setting
long measTime = 0;
long sendTime = 0;
long buzzerTime = 0;

//value setting
byte temperature = 0;
byte humidity = 0;

void setup() {
  Serial.begin(115200);
  WifiConnect();
  lcdSetting();
  pinMode(buzzer, OUTPUT);
}

void loop() {

  
  //每5秒讀取一次溫濕度
  if(millis() - measTime >= 5000){
    //讀取溫溼度
    ReadDHT(&temperature, &humidity);
    //顯示溫溼度
    lcdView(&temperature, &humidity);
    measTime = millis();
  }
  
  //溫度超過時,蜂鳴器開始,還在控制範圍時則5秒一次
  if((int)temperature >= 35 ||((int)temperature >= 29 && millis()- buzzerTime >=5000)){
    callBuzzer(&temperature);
    buzzerTime = millis();
  }
  
  //設定觸發LINE訊息條件為溫度超過29或濕度超過80
  if ((int)temperature >= 29 || (int)humidity >= 80) {
    if((int)temperature >= 35){
      sendLine(&temperature, &humidity);
    }else if(sendTime == 0 || millis()- sendTime >=300000){//5分鐘傳送一次
      sendLine(&temperature, &humidity);
    }
    
  }
  
}

//連線到指定的WiFi SSID
void WifiConnect(){
  Serial.print("Connecting Wifi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //連線成功，顯示取得的IP
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}

//讀取DHT11溫濕度
void ReadDHT(byte *temperature, byte *humidity) {
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(temperature, humidity, NULL)) !=
      SimpleDHTErrSuccess) {
    Serial.print("讀取失敗,錯誤訊息=");
    Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err));
    delay(1000);
    return;
  }
  Serial.print("DHT讀取成功：");
  Serial.print((int)*temperature); Serial.print(" *C, ");
  Serial.print((int)*humidity); Serial.println(" H");
}

void sendLine(byte *temperature, byte *humidity){
  //傳遞Line訊息
    String message = "溫溼度異常，目前環境狀態：";
    message += "\n溫度=" + String(((int)*temperature)) + " *C";
    message += "\n濕度=" + String(((int)*humidity)) + " H";
    Serial.println(message);
    //連線到Line API網站
    if (client.connect(host, 443)) {      
      int LEN = message.length();
      //1.傳遞網站
      String url = "/api/notify"; //Line API網址
      client.println("POST " + url + " HTTP/1.1");
      client.print("Host: "); client.println(host);//Line API網站      
      //2.資料表頭
      client.print("Authorization: Bearer "); client.println(Linetoken);
      //3.內容格式
      client.println("Content-Type: application/x-www-form-urlencoded");
      //4.資料內容
      client.print("Content-Length: "); client.println( String((LEN + 8)) ); //訊息長度      
      client.println();      
      client.print("message="); client.println(message); //訊息內容      
      //等候回應
      delay(2000);
      String response = client.readString();
      //顯示傳遞結果
      Serial.println(response);
      client.stop();//斷線，否則只能傳5次
      sendTime = millis();//紀錄上次傳送時間
    }
    else {
      //傳送失敗
      Serial.println("connected fail");
      sendTime = 0;//傳送失敗設0可重新傳送
    }
}

//設定LCD顯示內容
void lcdSetting(){
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Temperature:   c");
  lcd.setCursor(0, 1);
  lcd.print("Humidity:      %");
}

//顯示溫溼度在LCD上
void lcdView(byte *temperature, byte *humidity){
  lcd.setCursor(12, 0);
  lcd.print((int)*temperature);
  lcd.setCursor(12, 1);
  lcd.print((int)*humidity);
}

void callBuzzer(byte *temperature){
  if ((int)*temperature >= 35) {
    tone(buzzer, 4186, 50); //危險：發出高音Do
  }else if ((int)*temperature >= 29) {
    tone(buzzer, 523, 100); //小心：發出中音Do
  }
  delay(50);
}
