#define DEBUG

//------------------------------LIBRARY DECLARATION-----------------------------
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
// #include <LittleFS.h>
// #include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

//----------------------------VARIABLE DECLARATION AND DEFAULT VALUE---------------
// default value
// wifi config
char* wifi_ssid     = NULL;
char* wifi_password = NULL;
bool wifi_mode = false;
const int statusLED = 2;
unsigned long send_interval = 1000;


bool config_state = false;
bool req_reboot = false;
bool req_reset = false;


// web updater configuration
bool web_updater = false;
char* web_updater_url = NULL;
char* web_updater_token = NULL;


// mqtt configuration
WiFiClient espClient;
PubSubClient client(espClient);
bool mqtt = false;
char* mqtt_host = NULL;
char* mqtt_user = NULL;
char* mqtt_password = NULL;
char* mqtt_topic = NULL;
long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


// web server configuration
String ledState;
AsyncWebServer server(80);


// #include "webheader/bootstrap.css.h"
// #include "webheader/bootstrap.js.h"
// #include "webheader/jquerry.js.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//---------------------------------------CUSTOM FUNCTION INCLUDE-------------------
#include "helper.esp"
#include "configuration.esp"
#include "webserver.esp"
#include "wifi.esp"
#include "mqtt.esp"
//--------------------------------------SYSTEM MAIN SETUP-----------------------------
void ICACHE_FLASH_ATTR setup(){
  //......................................SENSOR SETUP..............................
  // pin and sensor setup
  Serial.begin(115200);
  Serial.println("Welcome To ESP_Frame Arduino!!");
  Serial.println("Setting up sensor and output pins...");
  pinMode(statusLED, OUTPUT);

  if(!SPIFFS.begin()){
#ifdef DEBUG
    Report("SPIFFS Error !!", 0);
#endif
    delay(1000);
    ESP.restart();
  }
  config_state = set_config();

  if(!config_state){
#ifdef DEBUG
    Report("Start first configuration..");
#endif
    wifi_fallback();
    webserver_fallback();
  }
  else{
    if(wifi_mode){
#ifdef DEBUG
      Serial.println("Setup Client!!");
      Serial.println(wifi_ssid);
      Serial.println(wifi_password);
#endif
      if(!wifi_client_setup(wifi_ssid, wifi_password)){
        wifi_set_to_fallback();
      }
    }
    else{
#ifdef DEBUG
      Serial.println("Setup AP!!");
      Serial.println(wifi_ssid);
      Serial.println(wifi_password);
#endif
      if(!wifi_AP_setup(wifi_ssid, wifi_password)){

      }
    }
    webserver();
  }

  //...............................MQTT SETUP...............................................
//   if(mqtt){
// #ifdef DEBUG
//     Serial.println("Mqtt Strated!!");
// #endif
//     client.setServer(mqtt_host, 1883);
//     client.setCallback(callback);
//   }
  
}

//-----------------------------SYSTEM LOOP--------------------------------------------------
void ICACHE_RAM_ATTR loop(){

}