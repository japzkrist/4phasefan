/*
  Simple Wifi Switch
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include <DNSServer.h>
#include <WiFiManager.h> 

// Replace with your network credentials
const char* ssid = "<ssid>";
const char* password = "<password>";

#define TRIGGER_PIN 0
#define LED 2

int gpio_12_relay = 12;
int gpio_13_relay = 13;
int gpio_14_relay = 14;

ESP8266WebServer server(80);

String web_1_html = "<h1>Switch 1 is ON</h1><p><a href=\"1\"><button>1</button></a>&nbsp;<a href=\"2\"><button>2</button></a>&nbsp;<a href=\"3\"><button>3</button></a>&nbsp;<a href=\"off\"><button>OFF</button></a></p>";
String web_2_html = "<h1>Switch 2 is ON</h1><p><a href=\"1\"><button>1</button></a>&nbsp;<a href=\"2\"><button>2</button></a>&nbsp;<a href=\"3\"><button>3</button></a>&nbsp;<a href=\"off\"><button>OFF</button></a></p>";
String web_3_html = "<h1>Switch 3 is ON</h1><p><a href=\"1\"><button>1</button></a>&nbsp;<a href=\"2\"><button>2</button></a>&nbsp;<a href=\"3\"><button>3</button></a>&nbsp;<a href=\"off\"><button>OFF</button></a></p>";
String web_off_html = "<h1>Switch is OFF</h1><p><a href=\"1\"><button>1</button></a>&nbsp;<a href=\"2\"><button>2</button></a>&nbsp;<a href=\"3\"><button>3</button></a>&nbsp;<a href=\"off\"><button>OFF</button></a></p>";

void setup(void){  
  //  Init
  WiFiManager wifiManager;
  //for testing
//  wifiManager.resetSettings();
  
  delay(10000);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  pinMode(gpio_12_relay, OUTPUT);
  digitalWrite(gpio_12_relay, HIGH);
  pinMode(gpio_13_relay, OUTPUT);
  digitalWrite(gpio_13_relay, HIGH);
  pinMode(gpio_14_relay, OUTPUT);
  digitalWrite(gpio_14_relay, HIGH);

  pinMode(TRIGGER_PIN, INPUT);
 
  Serial.begin(115200); 
  delay(5000);

  wifiManager.setTimeout(180);
    
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here "AP" and password "password"
  // and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("AP","password")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
  }    

  ota();

  webserver();

}
 
void loop(void){
  ArduinoOTA.handle();
  server.handleClient();

  // start the configuration portal on demand
  // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    Serial.println("Flash pin pressed");
    WiFi.disconnect(true);
    server.stop();
    
    WiFiManager wifiManager;
    
    if (!wifiManager.startConfigPortal("OnDemandAP","password")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    ota();

    webserver();

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
} 

void ota(){
  ArduinoOTA.setHostname("service02");
  ArduinoOTA.setPassword("F1rmw4r3");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void webserver(){
  server.on("/", [](){
    if(digitalRead(gpio_14_relay)==LOW) {
      server.send(200, "text/html", web_1_html);
    } else if(digitalRead(gpio_12_relay)==LOW) {
      server.send(200, "text/html", web_2_html);
    } else if(digitalRead(gpio_13_relay)==LOW) {
      server.send(200, "text/html", web_3_html);
    } else {
      server.send(200, "text/html", web_off_html);
    }
  });
  
  server.on("/1", [](){
    server.send(200, "text/html", web_1_html);
    digitalWrite(LED, LOW);
    digitalWrite(gpio_12_relay, HIGH);
    digitalWrite(gpio_13_relay, HIGH);
    delay(300);
    digitalWrite(gpio_14_relay, LOW);
    delay(1000);
  });
  server.on("/2", [](){
    server.send(200, "text/html", web_2_html);
    digitalWrite(LED, LOW);
    digitalWrite(gpio_12_relay, HIGH);
    digitalWrite(gpio_14_relay, HIGH);
    delay(300);
    digitalWrite(gpio_13_relay, LOW);
    delay(1000);
  });
  server.on("/3", [](){
    server.send(200, "text/html", web_3_html);
    digitalWrite(LED, LOW);
    digitalWrite(gpio_13_relay, HIGH);
    digitalWrite(gpio_14_relay, HIGH);
    delay(300);
    digitalWrite(gpio_12_relay, LOW);
    delay(1000);
  });
  
  server.on("/off", [](){
    server.send(200, "text/html", web_off_html);
    digitalWrite(LED, HIGH);
    digitalWrite(gpio_12_relay, HIGH);
    digitalWrite(gpio_13_relay, HIGH);
    digitalWrite(gpio_14_relay, HIGH);
    delay(1000); 
  });
  
  server.begin();
  Serial.println("Server ready..");
}
