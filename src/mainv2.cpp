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

// system states
bool config_state = false;
bool req_reboot = false;
bool req_reset = false;
bool user_stat = false;
bool net_state = false;

// web updater configuration
bool web_updater = false;
long web_updater_lastMsg = 0;
long web_updater_interval = 10000;
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
long last_mqtt_connect = 0;
long mqtt_interval = 1000;
long mqtt_lastMsg = 0;

#define MSG_BUFFER_SIZE	(100)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// web server configuration
String ledState;
AsyncWebServer server(8080);


//===================================Sensor Variables============================

const int led_1 = D1;
const int swtich_1 = D2; 
//===============================================================================

#include "webheader/bootstrap.css.h"
#include "webheader/bootstrap.js.h"
#include "webheader/jquerry.js.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//---------------------------------------CUSTOM FUNCTION INCLUDE-------------------
#include "user_main.esp"
#include "helper.esp"
#include "wifi.esp"
#include "mqtt.esp"
#include "configurationv2.esp"
#include "webserverv2.esp"
//--------------------------------------SYSTEM MAIN SETUP-----------------------------
void ICACHE_FLASH_ATTR setup(){
  // generate important variables
  if(mqtt_clientID == NULL || mqtt_clientID == ""){
    mqtt_clientID = strdup(gen_mqtt_clientid().c_str());
  }
  //......................................SENSOR SETUP..............................
  // pin and sensor setup
  Serial.begin(115200);
  Serial.println();
  Log("Welcome To ESP_Frame Arduino!!");
  pinMode(statusLED, OUTPUT);
  //=======================================PIN SETUP HERE==============================

  //=================================================================================

  if(!SPIFFS.begin()){
    Log("SPIFFS Error !!", 0);
    delay(1000);
    ESP.restart();
  }
  config_state = set_config();

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
      mqtt_client.setServer(mqtt_host, 1883);
      mqtt_client.setCallback(mqtt_callback);
    }
  }

  Log("Setting up user script", 3);
  user_stat = user_setup();
  if(user_stat){
    Log("User script failed to setup!", 0);
  }
  Log("System Started!!", 2);
}

//-----------------------------SYSTEM LOOP--------------------------------------------------
void ICACHE_RAM_ATTR loop(){
  if(req_reboot){
    delay(200);
    ESP.restart();
  }
  long time_now = millis();
  if(config_state){
    user_loop();
    if(net_state){
      //mqtt reconnect
      if(time_now - last_mqtt_connect >= 10000 && !mqtt_client.connected()){
        last_mqtt_connect = time_now;
        mqtt_reconnect();
      }
      if(mqtt_client.connected()){
        mqtt_client.loop();
      }

      //data sender
      //MQTT data sender
      if(time_now - mqtt_lastMsg >= mqtt_interval){
        mqtt_publish();
      }
      //web data sender
      if(time_now - web_updater_lastMsg >= web_updater_interval){
        web_sender();
      }
    }
  }
}