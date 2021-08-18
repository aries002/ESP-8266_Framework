///////////////////////////////////////////////////////////////////////////////////
/*
                Web Server Setup
  This file contains web server and web socket setup function. When need to add more
pages or new function you can edit this file and add the web server config on webserver function.

*/

///////////////////////////////////////////////////////////////////////////////////



//-------------------------------------SYSTEM WEB PROCESSOR--------------------------
void ICACHE_FLASH_ATTR handleConfig(AsyncWebServerRequest *request){
  
  
  Serial.println("Change configuration...");
  
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["configured"] = "true";
  
  if(request->hasParam("wifi_ssid", true)){
    root["wifi_ssid"] = request->getParam("wifi_ssid", true)
  }
  else{
    root["wifi_ssid"] = wifi_ssid;
  }
  if(request->hasParam("wifi_password", true)){
    root["wifi_password"] = request->getParam("wifi_password", true)
  }
  else{
    root["wifi_password"] = wifi_password;
  }
  if(request->hasParam("wifi_mode", true)){
    if(request->getParam("wifi_mode", true) == true){
      root["wifi_client"] = "true";
    }
    else{
      root["wifi_client"] = "false";
    }
  }
  else{
    root["wifi_client"] = "false";
  }
  if(request->hasParam("mqtt_host", true)){
    String mqtt_host = request->getParam("mqtt_host", true);
    if(mqtt_host != ""){
      root["mqtt_host"] = mqtt_host;
      if(request->hasParam("mqtt_user")){
        
      }
    }
  }
  else{
    root["mqtt_host"] = "";
  }
  if(request->hasParam("", true)){
    root[""] = request->getParam("", true)
  }
  else{
    root[""] = ;
  }

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


  // Delete old version
  SPIFFS.remove("/konfig.json");
  // Create the new one
  File f = SPIFFS.open("/konfig.json", "w");
  if (root.printTo(f) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  f.close();
  Serial.println("Configuration saved.");
  Serial.print("Device reboot in 3 seconds.");
  delay(3000);
  ESP.restart();
  if(request->hasParam(PARAM_MESSAGE, true))
}

String ICACHE_FLASH_ATTR getTemperature() {
  float temperature = 28.88;
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float temperature = 1.8 * bme.readTemperature() + 32;
  Serial.println(temperature);
  return String(temperature);
}
  
String ICACHE_FLASH_ATTR getHumidity() {
  float humidity = 80.88;
  Serial.println(humidity);
  return String(humidity);
}

String ICACHE_FLASH_ATTR getPressure() {
  float pressure = 34.00/ 100.0F;
  Serial.println(pressure);
  return String(pressure);
}
String ICACHE_FLASH_ATTR processor(const String& var){
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

void ICACHE_FLASH_ATTR webserver(){
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
    handleConfig(request);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", "Not found");
		request->send(response);
	});
  server.begin();
}

void ICACHE_FLASH_ATTR fallback_webserver(){

}