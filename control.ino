#include <ESP8266WiFi.h>        // 本程序使用 ESP8266WiFi库
#include <ESP8266WebServer.h>   //  ESP8266WebServer库
#include <DHT.h>
//io定义
#define FanIO  D6
#define TWIO  D2
#define DHTTYPE DHT11

const char* ssid     = "zcc333";                // 需要连接到的WiFi名
const char* password = "15144099398b";             // 连接的WiFi密码
ESP8266WebServer esp8266_server(3333);// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
IPAddress local_IP(192, 168, 0, 70); // 设置ESP8266-NodeMCU联网后的IP
IPAddress gateway(192, 168, 0, 1);    // 设置网关IP（通常网关IP是WiFI路由IP）
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(192,168,0,1);           // 设置局域网DNS的IP（通常局域网DNS的IP是WiFI路由IP
DHT dht(TWIO, DHTTYPE);
bool FanStatus=false;
bool IsHanderOn=false;
float tempGet=0.0;
//定时轮询
unsigned long previousMillis = 0;
unsigned long interval = 10000;
unsigned long disconnPreviousMillis = 0;
bool IsAutoFan=false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  initWifi();
  initWebServer();
  initDht();
  pinMode(FanIO,OUTPUT);
}
void initWifi(){
  // 设置开发板网络环境
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to ESP8266 IP"); 
    return;
  }
  WiFi.mode(WIFI_STA);                       // 设置Wifi工作模式为STA,默认为AP+STA模式
  WiFi.begin(ssid, password);
  int i = 0;                                    // 检查WiFi是否连接成功
  while (WiFi.status() != WL_CONNECTED)         // WiFi.status()函数的返回值是由NodeMCU的WiFi连接状态所决定的。 
  {                                             // 如果WiFi连接成功则返回值为WL_CONNECTED
    delay(1000);                                // 此处通过While循环让NodeMCU每隔一秒钟检查一次WiFi.status()函数返回值
    Serial.print("waiting...");                          
    Serial.print(i++);
    Serial.println('\n');
  }
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP
  // 设置当网络断开连接的时候自动重连
//  WiFi.setAutoReconnect(true);
//  WiFi.persistent(true);
}
void initWebServer(){
  //启动http服务
  esp8266_server.begin();                           
  esp8266_server.on("/off", handleFanOff); 
  esp8266_server.on("/on",  handleFanOn);
  esp8266_server.on("/auto",  handleAutoFan);
  esp8266_server.onNotFound(handleNotFound);
  Serial.println("HTTP esp8266_server started");//  告知用户ESP8266网络服务功能已经启动
}
void initDht(){
  dht.begin();
}
void handleAutoFan(){
  if (esp8266_server.hasArg("on")&&esp8266_server.arg("on") == "true") {
    IsAutoFan=true;
    esp8266_server.send(200, "text/plain", "智能监测开启");
  }else if(esp8266_server.hasArg("off")&&esp8266_server.arg("off") == "false"){
    IsAutoFan=false;
    digitalWrite(FanIO,LOW);
    esp8266_server.send(200, "text/plain", "智能监测关闭");
  }else{
    esp8266_server.send(200, "text/plain", "无效请求");
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
void handleNotFound(){                                              // 当浏览器请求的网络资源无法在服务器找到时，
  esp8266_server.send(404, "text/plain", "404: Not found");         // NodeMCU将调用此函数。
}
void GetTempAndWet(){
  if(!IsAutoFan){
    return;
   }
  //每隔10s读取
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
void DisConnCkeck(){
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - disconnPreviousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.RSSI());
    disconnPreviousMillis = currentMillis;
    }
  }
void loop() {
  //Serial.println(digitalRead(FanIO));
  esp8266_server.handleClient();
  //获取温湿度数据
  GetTempAndWet();
  DisConnCkeck();
}
