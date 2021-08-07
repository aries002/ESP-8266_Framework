#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
// #include <LittleFS.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// default value
// wifi config
const char* wifi_ssid     = "Almost Smart";
const char* wifi_password = "1234567890";
bool wifi_mode = false;
const int ledPin = 2;
int send_interval = 1000;
bool def_state;

// web updater configuration
bool web_updater = false;
const char* web_updater_url;
const char* web_updater_token;


// mqtt configuration
WiFiClient espClient;
PubSubClient client(espClient);
bool mqtt = false;
const char* mqtt_host;
const char* mqtt_user;
const char* mqtt_password;
const char* mqtt_token;
const char* mqtt_topic;
const char* PARAM_MESSAGE;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


// web server configuration
String ledState;
AsyncWebServer server(80);

//---------------------------------------SYSTEM MQTT CONNECTOR----------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


//-------------------------------------SYSTEM WEB PROCESSOR--------------------------
String getTemperature() {
  float temperature = 28.88;
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float temperature = 1.8 * bme.readTemperature() + 32;
  Serial.println(temperature);
  return String(temperature);
}
  
String getHumidity() {
  float humidity = 80.88;
  Serial.println(humidity);
  return String(humidity);
}

String getPressure() {
  float pressure = 34.00/ 100.0F;
  Serial.println(pressure);
  return String(pressure);
}
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  else if (var == "TEMPERATURE"){
    return "34";
  }
  else if (var == "HUMIDITY"){
    return "80";
  }
  else if (var == "PRESSURE"){
    return "1";
  }
  else{
    return "0";
  }
}

//------------------------------------SAVE CONFIGURATION-------------------------------
void saveConfig(){
  Serial.println("Saving configuration...");
  // Delete old version
  SPIFFS.remove("/konfig.json");
  // Create the new one
  File f = SPIFFS.open("/konfig.json", "w");
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["configured"] = "true";
  root["wifi_ssid"] = wifi_ssid;
  root["wifi_password"] = wifi_password;
  if(!wifi_mode){
    root["wifi_client"] = "false";
  }else{
    root["wifi_client"] = "false";
  }
  root["web_updater_url"] = web_updater_url;
  root["web_updater_token"] = web_updater_token;
  root["mqtt_host"] = mqtt_host;
  root["mqtt_user"] = mqtt_user;
  root["mqtt_password"] = mqtt_password;
  root["mqtt_token"] = mqtt_token;
  root["mqtt_topic"] = mqtt_topic;
  if (root.printTo(f) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  f.close();
  Serial.println("Configuration saved.");
  Serial.print("Device reboot in 3 seconds.");
  delay(3000);
  ESP.restart();
}

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
        def_state = false;
      }
      else{
        Serial.println("Configuration not configured.\nUsing default configuration.");
      }
    }
  }
  file_konfig.close();
  if(def_state){
    Serial.println("Configuration not preset...\nStarting first configuration setup.");
  }



  //....................................NETWORK SETUP....................................
  Serial.println("Setting up Network...");
  //if not true then ap mode set to AP mode
  if(!wifi_mode){
    Serial.println("Setting up Access Point.");
    Serial.print("SSID     : ");
    Serial.println(wifi_ssid);
    Serial.print("Password : ");
    Serial.println(wifi_password);
    WiFi.softAP(wifi_ssid, wifi_password);
    IPAddress IP = WiFi.localIP();
    Serial.println("AP created!!!");
    Serial.print("Device IP address: ");
    Serial.println(IP);
  }else{ //if true then device set to connect to existing wireless network
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);
    Serial.print("Conneting..");
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Wifi Connected.");
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
  }


  //....................................WEB SERVER SETUP............................
  // Web Server using main web service
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
    // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTemperature().c_str());
  });
  
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getHumidity().c_str());
  });
  
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getPressure().c_str());
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/konfig.html", String());
  });

  server.on("/save_config", HTTP_POST,  [](AsyncWebServerRequest *request){
    String message;
    if (request->hasParam(PARAM_MESSAGE, true)) {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", "Not found");
		request->send(response);
	});
  server.begin();

  //...............................MQTT SETUP...............................................
  if(mqtt){
    client.setServer(mqtt_host, 1883);
    client.setCallback(callback);
  }
  
}

//-----------------------------SYSTEM LOOP--------------------------------------------------
void loop(){
  if(mqtt){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }

  unsigned long now = millis();
  if (now - lastMsg > send_interval) {
    lastMsg = now;
    Serial.print("\n\n" + now);
    Serial.println(". Sending data");
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