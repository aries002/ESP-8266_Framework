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

// system states
bool config_state = false;
bool req_reboot = false;
bool req_reset = false;


// web updater configuration
bool web_updater = false;
char* web_updater_url = NULL;
char* web_updater_token = NULL;
long last_web_updater_send = 0;

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
long lastMsg = 0;
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
  Serial.println();
  Log("Welcome To ESP_Frame Arduino!!");
  Log("System Starting...", 2);
  pinMode(statusLED, OUTPUT);
  //=======================================PIN SETUP HERE==============================

  //=================================================================================

  if(!SPIFFS.begin()){
#ifdef DEBUG
    Log("SPIFFS Error !!", 0);
#endif
    delay(1000);
    ESP.restart();
  }
  config_state = set_config();

  if(!config_state){
#ifdef DEBUG
    Log("Start first configuration..", 3);
#endif
    wifi_fallback();
    webserver_fallback();
  }
  else{
    if(wifi_mode){
#ifdef DEBUG
      Log("Setup Client!!", 3);
      Log(wifi_ssid);
      Log(wifi_password);
#endif
      if(!wifi_client_setup(wifi_ssid, wifi_password)){
        wifi_set_to_fallback();
      }
    }
    else{
#ifdef DEBUG
      Log("Setup AP!!", 3);
      Log(wifi_ssid);
      Log(wifi_password);
#endif
      if(!wifi_AP_setup(wifi_ssid, wifi_password)){
      }
    }
    webserver();
    if(mqtt){
      mqtt_client.setServer(mqtt_host, 1883);
      mqtt_client.setCallback(mqtt_callback);
    }
  }
  // generate important variables
  if(mqtt_clientID == NULL || mqtt_clientID == ""){
    mqtt_clientID = strdup(gen_mqtt_clientid().c_str());
  }
  Log("System Started!!", 2);
}

//-----------------------------SYSTEM LOOP--------------------------------------------------
void ICACHE_RAM_ATTR loop(){
  if(req_reboot){
    delay(200);
    ESP.restart();
  }
  
  if(wifi_mode){
    long time_now = millis();
    //connect to mqtt broker
    if(mqtt){
      if(time_now - last_mqtt_connect >= 10000 && !mqtt_client.connected()){
        last_mqtt_connect = time_now;
        if(!mqtt_reconnect()){
        }
      }
      if(mqtt_client.connected()){
        mqtt_client.loop();
      }
    }

    //data sender
    if(time_now - lastMsg >= send_interval){
      lastMsg = time_now;
      if(mqtt && mqtt_client.connected()){

        ++value;
        snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        mqtt_client.publish(mqtt_pub_topic, msg);
      }
      if(web_updater){
        
      }
    }
  }
}