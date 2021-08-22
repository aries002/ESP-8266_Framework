void ICACHE_RAM_ATTR loop(){
  if(req_reboot){
#ifdef DEBUG
    Serial.println("Reboot Requested...");
    Serial.println("Reboot in 3 second..");
#endif
    delay(3000);
    ESP.restart();
  }
  if(req_reset){
    reset_config();
  }
  if(config_state && wifi_mode){
    if(mqtt){
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
    }
    unsigned long now = millis();
    if (now - lastMsg > send_interval) {
      lastMsg = now;
#ifdef DEBUG
      Serial.println();
      Serial.print('[' + now + ']');
      Serial.println("Sending data");
#endif
      if(web_updater){
#ifdef DEBUG
        Serial.println("Web sent");
#endif
      }
      
    }
  }

}