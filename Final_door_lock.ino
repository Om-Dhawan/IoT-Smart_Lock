
#include <HTTPClient.h>
#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProSwitch.h"

#define WIFI_SSID         "Kepler"    
#define WIFI_PASS         "244466666"
#define APP_KEY           "ae4b8d90-d8ff-4e90-b8c6-e8e765e9eb63"      
#define APP_SECRET        "8349fce2-acf3-45c7-819e-ef43a18172e5-0944b78c-e7e0-4298-b1b0-bcc49d8c46da"   
#define SWITCH_ID         "63d69e6c22e49e3cb5f312fc"  
#define BAUD_RATE         9600                // Change baudrate to your need
const char* host = "maker.ifttt.com";

#define BUZZER_PIN   2   // GPIO for LED (inverted)
#define Rled        5
#define Gled        21
bool myPowerState = false;
int a=1;
int val=0;

/* bool onPowerState(String deviceId, bool &state) 
 *
 * Callback for setPowerState request
 * parameters
 *  String deviceId (r)
 *    contains deviceId (useful if this callback used by multiple devices)
 *  bool &state (r/w)
 *    contains the requested state (true:on / false:off)
 *    must return the new state
 * 
 * return
 *  true if request should be marked as handled correctly / false if not
 */
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");
  myPowerState = state;
  return true; // request handled properly
}

// setup function for WiFi connection
void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

// setup function for SinricPro
void setupSinricPro() {
  // add device to SinricPro
  SinricProSwitch& mySwitch = SinricPro[SWITCH_ID];

  // set callback function to device
  mySwitch.onPowerState(onPowerState);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  //SinricPro.restoreDeviceStates(true); // Uncomment to restore the last known state from the server.
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  pinMode(BUZZER_PIN, OUTPUT); // define LED GPIO as output
  pinMode(Rled, OUTPUT);
  pinMode(Gled, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // turn off LED on bootup
  digitalWrite(Rled, LOW);
  digitalWrite(Gled, LOW);

  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
  WiFiClient client; 
  const int httpPort = 80;  
  if (!client.connect(host, httpPort)) {  
      Serial.println("connection failed");  
      return;
  } 
  val=hallRead();
//  Serial.println(val);
  delay(50);
  if(myPowerState){
    digitalWrite(Gled,HIGH);
      if(val>50){                // magnet is detected
        digitalWrite(BUZZER_PIN,LOW);
        digitalWrite(Rled,LOW);
  //      Serial.println("Sensor reactivated");
        a=1;     // reavtivate the sensor
        delay(200);
      }
      else{
          if(a){
            digitalWrite(BUZZER_PIN,HIGH);
            digitalWrite(Rled,HIGH);
            Serial.println("Breach Detected");
            String url = "/trigger/Breach/json/with/key/IwPAUyChOZ6gsbUNTpsrX";          //webhook link to trigger mail through IFTTT
            client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
                                    "Host: " + host + "\r\n" +   
                                    "Connection: close\r\n\r\n");    
            delay(5000);
            digitalWrite(BUZZER_PIN,LOW);      // turn of buzzer after 5 secs
            digitalWrite(Rled,LOW);
            a=0;        // turns off/deactivate the sensor
            delay(20);
          }
          else{
            a=0;
          }
      }
  } 
  else{
    digitalWrite(Gled,LOW);
  }
}
