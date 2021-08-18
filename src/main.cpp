//------------------------------LIBRARY DECLARATION-----------------------------

#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
// #include <LittleFS.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


//----------------------------VARIABLE DECLARATION AND DEFAULT VALUE---------------
// default value
// wifi config
const char* wifi_ssid     = "Almost Smart";
const char* wifi_password = "1234567890";
bool wifi_mode = false;
const int ledPin = 2;
int send_interval = 1000;
bool config_state = false;
bool req_reboot = false;

// web updater configuration
bool web_updater = false;
const char* web_updater_url = "";
const char* web_updater_token = "";


// mqtt configuration
WiFiClient espClient;
PubSubClient client(espClient);
bool mqtt = false;
const char* mqtt_host = "";
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_topic = "";
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


// web server configuration
String ledState;
AsyncWebServer server(80);




////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//---------------------------------------CUSTOM FUNCTION INCLUDE-------------------
#include "webserver.esp"
#include "wifi.esp"
#include "mqtt.esp"
//--------------------------------------SYSTEM MAIN SETUP-----------------------------
void setup(){
  //......................................SENSOR SETUP..............................
  // pin and sensor setup
  Serial.begin(115200);
  Serial.println("Welcome To ESP_Frame Arduino!!");
  Serial.println("Setting up sensor and output pins...");
  pinMode(ledPin, OUTPUT);

  // ................................GET CONFIGURATION.............................
  Serial.println("Checking Configuration");
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File file_konfig = SPIFFS.open("/konfig.json", "r");
  if(file_konfig){
    size_t size = file_konfig.size();
    if (size > 1024){
      Serial.println("Config size to big");
    }
    std::unique_ptr<char[]> buf(new char[size]);
    //read configuration file
    file_konfig.readBytes(buf.get(), size);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& json_konfig = jsonBuffer.parseObject(buf.get());
    if (!json_konfig.success()) {
      Serial.println("Failed to parse config file");
    }else{
      //check if device configured
      if(json_konfig["configured"] == "true"){
        Serial.println("Get configuration");
        wifi_ssid = json_konfig["wifi_ssid"];
        wifi_password = json_konfig["wifi_password"];
        // check wifi mode is AP or Client
        if (json_konfig["wifi_client"] == "true"){
          wifi_mode = false;
          Serial.println("Wifi Mode : Access Point");
        }
        else{
          wifi_mode = true;
          Serial.println("Wifi Mode : CLient");
        }
        Serial.print("Wifi SSID : ");
        Serial.println(wifi_ssid);
        // set default configuration status to false
        config_state = true;
      }
      else{
        Serial.println("Configuration not configured.\nUsing default configuration.");
      }
    }
  }
  file_konfig.close();

  if(!config_state){
    Serial.println("Start first configuration..");
    wifi_AP_setup(wifi_ssid, wifi_password);
    fallback_webserver();
  }
  else{
    wifi_client_setup(wifi_ssid, wifi_password);
    webserver();
  }

  //...............................MQTT SETUP...............................................
  if(mqtt){
    client.setServer(mqtt_host, 1883);
    client.setCallback(callback);
  }
  
}

//-----------------------------SYSTEM LOOP--------------------------------------------------
void loop(){
  if(req_reboot){
    Serial.println("Reboot Requested...");
    Serial.println("Reboot in 3 second..");
    delay(3000);
    ESP.restart();
  }
  if(!config_state){
    if(mqtt){
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
    }
    unsigned long now = millis();
    if (now - lastMsg > send_interval) {
      lastMsg = now;
      Serial.println();
      Serial.print('[' + now + '] ');
      Serial.println("Sending data");
      if(mqtt){
        Serial.println("Mqtt sent");
        snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld");
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish(mqtt_topic, msg);
      }
      if(web_updater){
        Serial.println("Web sent");
      }
      
    }
  }

}