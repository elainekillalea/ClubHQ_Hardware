#include "secrets.h"
#include <WiFiClientSecure.h>     //aws
#include <PubSubClient.h>         //aws
#include <ArduinoJson.h>        
#include <Adafruit_Fingerprint.h> //fps
#include "WiFi.h"                 //esp32 lib
#include "time.h"                 //esp32 lib
 
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// pin #16 is IN from sensor (GREEN wire)
// pin #17 is OUT from arduino  (WHITE wire)
SoftwareSerial mySerial(16, 17);
#else
// ESP32 only has Serial0(don't use) and Serial2
#define mySerial Serial2
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial, 0);
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

const char* ntpServer = "pool.ntp.org";     //cluster of timeservers
const long  gmtOffset_sec = 0;              //offset in s for non-GMT timezones
const int   daylightOffset_sec = 0; // 3600;      //offset for daylight savings 3600s = 1hr

/********************************************** AWS **********************************************/
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
/********************************************** PUB **********************************************/
void publishMessage(int fid)
{
  StaticJsonDocument<200> doc;
  doc["fingerID"] = fid;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
/********************************************** SUB **********************************************/
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}
/********************************************** FPS **********************************************/
void connectFPS()
{
  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  // if (finger.verifyPassword()) {
  //   Serial.println("Found fingerprint sensor!");
  // } else {
  //   Serial.println("Did not find fingerprint sensor :(");
  //   // while (1) { delay(1); }
  // }

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please upload enroll code.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); 
    Serial.print(finger.templateCount); 
    Serial.println(" templates");
  }
}
/**********************************************   **********************************************/
void setup()
{
  Serial.begin(115200);
  Serial.println("Please connect to Wi-Fi!");
  connectAWS();
  connectFPS();
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
 
void loop()
{ 
  getFingerprintIDez();
  
  client.loop();
  delay(500);
}

/********************************************** FPS **********************************************/
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  publishMessage(finger.fingerID);
  printLocalTime();
  
  return finger.fingerID;
}


void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("Date: ");
  Serial.print(&timeinfo, "%d");
  Serial.print("-");
  Serial.print(&timeinfo, "%B");
  
  Serial.print("-");
  Serial.println(&timeinfo, "%Y");

  Serial.println();
}
