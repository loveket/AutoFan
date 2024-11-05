#include <ESP8266WiFi.h>        // 本程序使用 ESP8266WiFi库
#include <ESP8266WiFiMulti.h>   //  ESP8266WiFiMulti库
#include <ESP8266WebServer.h>   //  ESP8266WebServer库
#include <DHT.h>
//io定义
#define FanIO  D6
#define TWIO  D2
#define DHTTYPE DHT11
ESP8266WiFiMulti wifiMulti;     // 建立ESP8266WiFiMulti对象,对象名称是'wifiMulti'
ESP8266WebServer esp8266_server(3333);// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
IPAddress local_IP(192, 168, 0, 70); // 设置ESP8266-NodeMCU联网后的IP
IPAddress gateway(192, 168, 0, 1);    // 设置网关IP（通常网关IP是WiFI路由IP）
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(192,168,0,1);           // 设置局域网DNS的IP（通常局域网DNS的IP是WiFI路由IP
DHT dht(TWIO, DHTTYPE);
bool FanStatus=false;
bool IsHanderOn=false;
float tempGet=0.0;
unsigned long previousMillis = 0;
unsigned long interval = 15000;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //初始化http服务
  initWebServer();
  pinMode(FanIO,OUTPUT);
  dht.begin();
}
void initWebServer(){
  // 设置开发板网络环境
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to ESP8266 IP"); 
    return;
  }
  wifiMulti.addAP("zcc333", "15144099398b"); // 将需要连接的一系列WiFi ID和密码输入这里
  Serial.println("Connecting ..."); 
  int i = 0;
  bool isExe=false;                                 
  while (wifiMulti.run() != WL_CONNECTED) {  // 此处的wifiMulti.run()是重点。通过wifiMulti.run()，NodeMCU将会在当前
    delay(1000);                             // 环境中搜索addAP函数所存储的WiFi。如果搜到多个存储的WiFi那么NodeMCU
    Serial.print(i++); Serial.print(' ');    // 将会连接信号最强的那一个WiFi信号。
    if(i>=5){
      isExe=true;
      break;
     }
  }                                          // 一旦连接WiFI成功，wifiMulti.run()将会返回“WL_CONNECTED”。这也是
                                             // 此处while循环判断是否跳出循环的条件。
  if(!isExe){
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP
   // 设置当网络断开连接的时候自动重连
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  esp8266_server.begin();                           // 启动网站服务
  esp8266_server.on("/off", handleFanOff);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/on",  handleFanOn);  // 设置处理LED控制请求的函数'handleLED'
  Serial.println("HTTP esp8266_server started");//  告知用户ESP8266网络服务功能已经启动
   }
  
}
void handleFanOff(){
  if(!FanStatus){
    esp8266_server.send(200, "text/plain", "风扇已经关闭");
    return;
    }
  Serial.println("off");
  digitalWrite(FanIO,LOW);
  FanStatus=false;
  IsHanderOn=false;
  esp8266_server.send(200, "text/plain", "shut fan");
}
void handleFanOn(){
  if(FanStatus){
    esp8266_server.send(200, "text/plain", "风扇已经打开");
    return;
    }
  Serial.println("on");
   digitalWrite(FanIO,HIGH);
   FanStatus=true;
   IsHanderOn=true;
   esp8266_server.send(200, "text/plain", "open fan");
}
void GetTempAndWet(){
  //每隔15s读取
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >=interval){
    //wetGet = dht.readHumidity();//读湿度
    tempGet = dht.readTemperature();//读温度，默认为摄氏度
    Serial.println(tempGet);
    previousMillis = currentMillis;
    if(IsHanderOn){
      return;
    }
    if(tempGet>=28.0&&!FanStatus){
      digitalWrite(FanIO,HIGH);
      FanStatus=true;
    }
    if(tempGet<28.0&&FanStatus){
      digitalWrite(FanIO,LOW);
      FanStatus=false;
    }
    
   }
 }
void loop() {
  //Serial.println(digitalRead(FanIO));
  //delay(10000);
  esp8266_server.handleClient();
  //获取温湿度数据
  GetTempAndWet();
}
