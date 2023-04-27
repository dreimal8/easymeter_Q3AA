#include <Arduino.h>
#include "variablen.h" // Variablen
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "myOTA.h"


unsigned long mqttUpdateTime;
bool mqttok = false;
const char* mqtt_pub = "easy_meter_2";
const char* mqtt_sub = "empfangen";

WiFiClient wificlient;
PubSubClient psclient(wificlient);

int sendCount = 1;
int sendInterval = 3;

#include <SoftwareSerial.h>

#define MYPORT_TX 5  // D1
#define MYPORT_RX 4  // D2

SoftwareSerial myPort;

uint32_t prevOneSecMillis = 0;
uint32_t prev_smlMillis = 0;

const String smlBegin               = "1b1b1b1b01010101";
const String smlEnd                 = "1b1b1b1b1a";
const String searchStr_Bezug        = "77070100010800ff"; //Zählerstand Verbrauch
const String searchStr_Einspeisung  = "77070100020800ff"; //Zählerstand Einspeisung
const String searchStr_Leistung     = "77070100100700ff"; //Aktuelle Leistung

String smlTemp = ""; 
String smlMsg = "";

float bezug = 0.0;
float einspeisung = 0.0;
float leistung = 0.0;

bool foundStart = false;
bool foundEnd = false;
int indexBegin = 0;
int indexEnd = 0;

uint32_t currentMillis;

bool toggleLED = true;



//##########################################################################
//#                 bytetoHEX
//##########################################################################
String bytetoHEX(byte onebyte) {
  String str = "";
  if (onebyte < 16) str += String(0, HEX);
  str += String(onebyte, HEX);
  return str;
}


//##########################################################################
//#                 parse_smlMsg
//##########################################################################
void parse_smlMsg() {
  
  int64_t value = 0;
  String hexStr = "";

  // bezug (Wh)
  String searchStr = searchStr_Bezug;
  uint16_t pos = smlMsg.indexOf(searchStr);
  if (pos > 0) {
    pos = pos + searchStr.length() + 20;   // skip additional 10 Bytes = 20 Char!
    hexStr = smlMsg.substring(pos,  pos + 16);   // hexStr is 8 Byte = 16 Char
    value = strtoull(hexStr.c_str(), NULL, 16);
    bezug = (float)value;
  } else {
    bezug = 0.0;
  }

  // einspeisung (Wh)
  searchStr = searchStr_Einspeisung;
  pos = smlMsg.indexOf(searchStr);
  if (pos > 0) {
    pos = pos + searchStr.length() + 20;
    hexStr = smlMsg.substring(pos,  pos + 16);
    value = strtoull(hexStr.c_str(), NULL, 16);
    einspeisung = (float)value;
  } else {
    einspeisung = 0.0;
  }

  // leistung (W)
  searchStr = searchStr_Leistung;
  pos = smlMsg.indexOf(searchStr);
  if (pos > 0) {
    pos = pos + searchStr.length() + 14;
    hexStr = smlMsg.substring(pos,  pos + 16);
    value = strtoull(hexStr.c_str(), NULL, 16);
    leistung = (float)value;
  } else {
    leistung = 0.0;
  }

}


//##########################################################################
//#                 SMLoutput
//##########################################################################
void SMLoutput() {

  smlTemp = smlTemp.substring(indexBegin, indexEnd + smlEnd.length());
  
  smlMsg = smlTemp;
  Serial.println("start");
  Serial.println(smlMsg);
  Serial.println("end");
  Serial.println(currentMillis - prev_smlMillis);
  prev_smlMillis = currentMillis;
  parse_smlMsg();
  Serial.println("bezug = " + String(bezug, 1) + " Wh");
  Serial.println("einspeisung = " + String(einspeisung, 1) + " Wh");
  Serial.println("leistung = " + String(leistung, 1) + " W");

  if (toggleLED) {
    digitalWrite(LED_BUILTIN, HIGH);
    toggleLED = false;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    toggleLED = true;
  }

  if ( sendCount == sendInterval ) {
    sendMQTT( String( String(leistung) + "|" + String(bezug) + "|" + String(einspeisung) ), String("Datensatz") );
    sendCount = 0;
  }
  sendCount = sendCount + 1;
  
  Serial.println("");
  
  resetValues();
  
}


//##########################################################################
//#                 resetValues
//##########################################################################
void resetValues() {

  indexBegin = 0;
  indexEnd = 0;
  foundStart = false;
  foundEnd = false;
  smlTemp = "";                  // start with empty temporary SML message
  prevOneSecMillis = currentMillis;
  
}


//##########################################################################
//#                  LED blinker für Diagnose
//##########################################################################
void blink(int an, int aus, int anzahl){

  while(anzahl>0){
    digitalWrite(LED_BUILTIN, LOW);
    delay(an);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(aus);
    anzahl = anzahl - 1;
  }
}


//##########################################################################
//#                      reconnects to MQTT broker
//##########################################################################
void reconnect() {

  int count = 0;
  
  // Loop until we're reconnected
  while (!psclient.connected()) {
    Serial.print("MQTT Verbindungsaufbau...");
    if (psclient.connect(mqtt_pub)) {
      Serial.println("verbunden");
      mqttok = true;  
      // Subscribe or resubscribe to a topic
      //psclient.subscribe(mqtt_pub + "/" + mqtt_sub);
    } else {
      count = count + 1;
      Serial.print("Verbindungsversuch "); 
      Serial.print(count); 
      Serial.println(" fehlgeschlagen");
      if (count==3) {
        Serial.println("Abbruch - Verbindung nicht möglich");
        mqttok = false;
        return;
      }      
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(psclient.state());
      Serial.println(" neuer Versuch in 3 Sekunden");
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
}


//##########################################################################
//#                      sendMQTT
//##########################################################################
void sendMQTT(String wert, String thema) {

  if (!psclient.connected()) {
    reconnect();
  }

  if (mqttok){
    
    int str_len = wert.length() + 1;
    char char_array_wert[str_len];
    wert.toCharArray(char_array_wert, str_len);

    String topic = String(mqtt_pub) + "/" + String(thema);
    int str_len2 = topic.length() + 1;
    char char_array_thema[str_len2];
    topic.toCharArray(char_array_thema, str_len2);    
    
    if(!psclient.loop())
    psclient.connect(mqtt_pub);
    psclient.publish(char_array_thema, char_array_wert);
    Serial.print(topic);
    Serial.print("   ");
    Serial.print(wert);
    Serial.println("    MQTT gesendet...");
  }
}


//##########################################################################
//#                          connectToWifi
//##########################################################################
void connectToWifi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int count = 0;
  
  Serial.print("WiFi Verbindung wird aufgebaut");
  while (WiFi.status() != WL_CONNECTED) {
    blink(100, 400, 1);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Verbunden, IP Adresse: ");
  Serial.println(WiFi.localIP());
  blink(1000, 1000, 2);

}


//##########################################################################
//#                  SETUP
//##########################################################################
void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT); // LED als Output definieren
  digitalWrite(LED_BUILTIN, HIGH); // Ausschalten
  
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Setup");

  myPort.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (myPort) { // If the object did not initialize, then its configuration is invalid
    Serial.println("SoftwareSerial port initialized.");
  } else {
    Serial.println("Invalid SoftwareSerial pin configuration, check config!"); 
  }
  delay(2000);

  connectToWifi();
  psclient.setServer(mqtt_server, 1883);

  initOTA();

}


//##########################################################################
//#                     LOOP
//##########################################################################
void loop() {

  // put your main code here, to run repeatedly:
    if (!psclient.connected()) {
      Serial.print("mqtt reconnect gerufen...");
      reconnect();
    }
    if(!psclient.loop()) {
      Serial.print("loop failed");
      psclient.connect(mqtt_pub);
    }

    ArduinoOTA.handle();            //OTA Update


   /********************************************************************************************
   * Stromzähler lesen 
   ********************************************************************************************/

  currentMillis = millis();
  uint8_t inByte;
 
  while (myPort.available()) {

    inByte = myPort.read(); // read serial buffer into array
    smlTemp += bytetoHEX(inByte);
    //Serial.print(bytetoHEX(inByte));
    indexBegin = smlTemp.indexOf(smlBegin);
    if (indexBegin > 0) {
      if (!foundStart) Serial.println(smlBegin + " gefunden! - Start");
      foundStart = true;
      indexEnd = smlTemp.lastIndexOf(smlEnd);
      if (indexEnd > indexBegin) {                     // end of temporary SML message reached and complete now
        Serial.println(smlEnd + " gefunden! - End");
        foundEnd = true;
        //Serial.println(smlTemp);
        SMLoutput();
      }
    }

  }


  if (currentMillis - prevOneSecMillis >= 3200) {

    prevOneSecMillis = currentMillis;
    Serial.println("timeout - reset");
    myPort.end();
    resetValues();
    myPort.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
    blink(100, 100, 5);
    sendMQTT( String("timeout..."), String("Info") );
    delay(100);

  }

  delay(10);

}
