#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <DHT.h>
#include <MsTimer2.h>

//#define DEBUG
//#define DEBUG_WIFI
#define AP_SSID "iot0"
#define AP_PASS "iot00000"
#define SERVER_NAME "10.10.141.72"
#define SERVER_PORT 5000
#define LOGID "ARD"
#define PASSWD "PASSWD"
#define CMD_SIZE 50
#define DHT_PIN 4
#define DHTTYPE DHT11
#define ARR_CNT 6
#define WIFIRX 6  //6:RX-->ESP8266 TX
#define WIFITX 7  //7:TX -->ESP8266 RX
#define TRIG 9 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 //ECHO 핀 설정 (초음파 받는 핀)

SoftwareSerial wifiSerial(WIFIRX, WIFITX);
WiFiEspClient client;
DHT dht(DHT_PIN, DHTTYPE);

bool timerFlag = false;
long duration, distance;
float tem = 0.0, hum = 0.0;
int res = 0;
int cds = 0;
char sendId[10] = "ARD";
char sendSensorVal[50] = {0};
char getSensorId[10]="SQL";
char getSensorId1[10]="BLT";
char display[50];
char temp[6];
char humi[6];

void timerIsr()
{
  timerFlag = true;
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void socketEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len;

  sendSensorVal[0] = '\0';
  len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif
  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  //[KSH_ARD]LED@ON : pArray[0] = "KSH_ARD", pArray[1] = "LED", pArray[2] = "ON"
  if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    strcpy(display, "Server Connected");
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    client.stop();
    server_Connect();
    return ;
  }
  else
    return;
   

  client.write(sendSensorVal, strlen(sendSensorVal));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendSensorVal);
#endif
}

int server_Connect()
{
#ifdef DEBUG_WIFI
  Serial.println("Starting connection to server...");
#endif

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI
    Serial.println("Connect to server");
#endif
    client.print("["LOGID":"PASSWD"]");
  }
  else
  {
#ifdef DEBUG_WIFI
    Serial.println("server connection failure");
#endif
  }
}

void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI
      Serial.println("WiFi shield not present");
#endif
    }
    else
      break;
  } while (1);

#ifdef DEBUG_WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
  }

#ifdef DEBUG_WIFI
  Serial.println("You're connected to the network");
  printWifiStatus();
#endif
}

void wifi_Setup() {
  wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}



void setup() {

  Serial.begin(19200); 
  #ifdef DEBUG
  Serial.begin(115200); //DEBUG
  #endif
  wifi_Setup();
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  MsTimer2::set(30000, timerIsr); //30초 간격으로 타이머 작동
  MsTimer2::start();
  dht.begin();
}

void loop()
{
  if (client.available()) {
    socketEvent();
  }

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(3);
  digitalWrite(TRIG, LOW);

  duration = pulseIn (ECHO, HIGH); 
  distance = duration * 17 / 1000; 

  if(distance > 400){
    distance = 1;
  }

  tem = dht.readTemperature();
  hum = dht.readHumidity();

 
    Serial.print(distance); 
    Serial.println(" Cm");
    cds = analogRead(A1);
    Serial.print(" Illumination : ");
    Serial.println(cds);

  sprintf(temp,"%s",String(tem,2).c_str());
  Serial.print(temp);
  
  sprintf(humi,"%s",String(hum,2).c_str());
  Serial.println(humi);
  if (timerFlag) //30초에 한번씩 실행
  {
    timerFlag = false;
    sprintf(sendSensorVal, "[%s]SENSOR@%d@%s@%s@%d\r\n", getSensorId, cds, temp, humi, distance);
    client.write(sendSensorVal, strlen(sendSensorVal));
    delay(100);
    sprintf(sendSensorVal, "[%s]SENSOR@%d@%s@%s@%d\r\n", getSensorId1, cds, temp, humi, distance);
    client.write(sendSensorVal, strlen(sendSensorVal));
    client.flush();
  }
}