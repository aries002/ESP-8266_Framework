#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>

// default value
// wifi config
const char* wifi_ssid     = "Almost Smart";
const char* wifi_password = "1234567890";
const char* wifi_mode = "AP";

WiFiServer server(80);
int sensor_interval = 1000;
const char* main_config = "True";

bool web_updater = true;
bool mqtt = true;
void setup(){
  Serial.begin(115200);
  Serial.println("Welcome To ESP_Frame Arduino!!");
  Serial.println("Checking Configuration");
  //check if config.json is present
  if(main_config == "True"){
    // if configuration present and falid
    // read and copy all config to main variables
    // setup WiFi
    if(wifi_mode == "STA"){
      
    }
    else if(wifi_mode == "APS"){
      
    }
    else{
      Serial.println("Setting up Access Point.");
      WiFi.softAP(wifi_ssid, wifi_password);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("Device IP address: ");
      Serial.println(IP);
      
    }
    
    // Web Server using main web service
  }
  else{
    // if config not present start new configuration wizard
    Serial.println("Configuration not preset...\nStarting first configuration setup.");
    // AP Setup
    Serial.println("Bringing UP Access Point..,");
    // Web Server configuration using first setup web pages
  }
  // pin and sensor setup
  Serial.println("Setting up sensor and output pins...");
}

void loop(){
  // delay for sensor reading intervals
  delay(sensor_interval);
  // sensor reading
  // check if using web service updater
  
  if(web_updater){
    // if web updater configured
    // send data to web server
    Serial.println("Sensor value sent..");
  }
  // check if using mqtt
  if(mqtt){
    
  }

}
