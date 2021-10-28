#define DEBUG
#define logLevel 2
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
#include <LiquidCrystal_I2C.h>
//----------------------------VARIABLE DECLARATION AND DEFAULT VALUE---------------
// default value
// wifi config
char* wifi_ssid     = NULL;
char* wifi_password = NULL;
bool wifi_mode = false; // IF TRUE USING CLIENT

// system variable
#ifdef ESP8266
const int statusLED = 2;
#endif
#ifndef ESP8266
const int statusLED = 2;
#endif
long send_interval = 5000;
const int rst_pin = 0;
char* system_name = "ESPFrame";
unsigned long time_now = 0;
// system states
bool config_state = false;
bool req_reboot = false;
bool req_reset = false;
bool user_stat = false;
bool net_state = false;

// web updater configuration
bool web_updater = false;
unsigned long web_updater_lastMsg = 0;
unsigned long web_updater_interval = 10000;
char* web_updater_url = NULL;
char* web_updater_token = NULL;

// mqtt configuration
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
bool mqtt = false;
char* mqtt_host = NULL;
char* mqtt_user = NULL;
char* mqtt_password = NULL;
char* mqtt_topic = NULL;
char* mqtt_clientID = NULL;
char* mqtt_pub_topic = NULL;
char* mqtt_sub_topic = NULL;
unsigned long last_mqtt_connect = 0;
unsigned long mqtt_interval = 10000;
unsigned long mqtt_lastMsg = 0;

#define MSG_BUFFER_SIZE	(512)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// web server configuration
String ledState;
AsyncWebServer server(8080);


//===================================Sensor Variables============================

const int led_1 = D1;
const int swtich_1 = D2; 
//===============================================================================

#include "webheader/bootstrap.min.css.h"
#include "webheader/bootstrap.min.js.h"
#include "webheader/jquery.js.h"
#include "webheader/first_config.h"
// #include "webheader/bootstrap.min.css.map.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//---------------------------------------CUSTOM FUNCTION INCLUDE-------------------
#include "helper.esp"
#include "application.esp"
#include "wifi.esp"
#include "mqtt.esp"
#include "configuration.esp"
#include "webserver.esp"
//--------------------------------------SYSTEM MAIN SETUP-----------------------------
void ICACHE_FLASH_ATTR setup(){
  // generate important variables
  
  //......................................SENSOR SETUP..............................
  // pin and sensor setup
  Serial.begin(115200);
  Serial.println();
  Log("Welcome To ESP_Frame Arduino!!");
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, LOW);
  //=======================================PIN SETUP HERE==============================

  //=================================================================================

  if(!SPIFFS.begin()){
    Log("SPIFFS Error !!", 0);
    delay(1000);
    ESP.restart();
  }
  config_state = set_config();
  if(mqtt_clientID == NULL || mqtt_clientID == ""){
    mqtt_clientID = strdup(gen_mqtt_clientid().c_str());
  }
  if(!config_state){
    Log("Start first configuration..", 3);
    wifi_fallback();
    webserver_fallback();
  }
  else{
    if(wifi_mode){
      Log("Setup Client!!", 3);
      Log(wifi_ssid, 3);
      Log(wifi_password, 3);
      net_state = wifi_client_setup(wifi_ssid, wifi_password);
      if(!net_state){
        wifi_set_to_fallback();
      }
    }
    else{
      Log("Setup AP!!", 3);
      Log(wifi_ssid, 3);
      Log(wifi_password, 3);
      if(!wifi_AP_setup(wifi_ssid, wifi_password)){
      }
    }
    webserver();
    if(mqtt && net_state){
      mqtt_connect();
    }
  }

  Log("Setting up user script", 3);
  if(config_state){
    user_stat = user_setup();
    if(!user_stat){
      Log("User script failed to setup!", 0);
    }
  }
  Log("System Started!!", 2);
}

//-----------------------------SYSTEM LOOP--------------------------------------------------
void IRAM_ATTR loop(){
  time_now = millis();
  
  if(req_reboot){
    delay(200);
    ESP.restart();
  }
  // if(!config_state){
  //   Log("CONFIG STATE FALSE", 3);
  // }
  if(config_state){
    user_loop();
    if(net_state){
      //mqtt reconnect
      if(mqtt){
        //if mqtt connection lost try to connect immidietly atleast every 30 second
        if(time_now - last_mqtt_connect >= 30000 && !mqtt_client.connected()){
          Log("Reconnecting to MQTT Broker");
          last_mqtt_connect = time_now;
          mqtt_reconnect();
        }
        if(mqtt_client.connected()){
          mqtt_client.loop();
          //send mqtt data
          if(time_now - mqtt_lastMsg >= mqtt_interval){
            Log("Publishing MQTT data",3);
            mqtt_publish();
            mqtt_lastMsg = time_now;
          }
        }
      }

      //web data sender
      if(time_now - web_updater_lastMsg >= web_updater_interval){
        web_sender();
      }
    }
  }
  breathe(statusLED);
}