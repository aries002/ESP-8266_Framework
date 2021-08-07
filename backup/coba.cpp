void saveconfig(){
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
  root["wifi_client"] = ;
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



void saveConfig() {
  Serial.println("Saving configuration...");
  // Delete old version
  SPIFFS.remove("/konfig.json");
  // Create the new one
  File f = SPIFFS.open("/konfig.json", "w");

  String json_txt = "{\n";
  json_txt += "\"configured\" : \"true\",\n";
  json_txt += "\"wifi_ssid\" : \"" + wifi_ssid +"\",\n";
  json_txt += "\"wifi_password\" : \"" + wifi_password + "\",\n";
  if(!wifi_mode){
    json_txt += "\"wifi_client\" : \"false\",\n";
  }else{
    json_txt += "\"wifi_client\" : \"true\",\n";
  }
  // json_txt += "\"wifi_client\" : \""+wifi_client+"\",\n";
  json_txt += "\"web_updater_url\" : \"" + web_updater_url + "\",\n";
  json_txt += "\"web_updater_token\" : \"" + web_updater_token +"\",\n";
  json_txt += "\"mqtt_host\" : \"" + mqtt_host + "\",\n";
  json_txt += "\"mqtt_user\" : \"" + mqtt_user+"\",\n";
  json_txt += "\"mqtt_password\" : \"" + mqtt_password + "\",\n";
  json_txt += "\"mqtt_token\" : \"" + mqtt_token + "\",\n";
  json_txt += "\"mqtt_topic\" : \"" + mqtt_topic + "\"\n";
  json_txt += "}";
  Serial.println(json_txt);
  f.print(json_txt);
  f.close();

  Serial.println("Configuration saved.");
  Serial.print("Device reboot in 3 seconds.");
  delay(3000);
  ESP.restart();
}