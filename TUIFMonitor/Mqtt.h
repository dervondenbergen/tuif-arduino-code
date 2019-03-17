
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {

  for (int i = 0; i < departurecount; i++) {
    mqttClient.subscribe( departures[i].key.c_str() , 0);
  }
  
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  const size_t capacity = JSON_ARRAY_SIZE(3);
  StaticJsonDocument<capacity> doc;

  auto error = deserializeJson(doc, payload);
  if (error) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      return;
  }
  
  JsonArray root = doc.as<JsonArray>();

  for (int i = 0; i < departurecount; i++) {
    const char* k = departures[i].key.c_str();
    
    if (strcmp(k, topic) == 0) {
      departures[i].time0 = root[0];
      departures[i].time1 = root[1];
    }
  }

  String currentTopic;
  Items ci = items[currentItem];
  if (ci.type == wienerlinien) {
    int ad = wl[ci.index].activeDirection;
    currentTopic = "wl/" + String(wl[ci.index].keys[ad]) + "/" + wl[ci.index].line;
  }
  if (ci.type == citybike) {
    currentTopic = "cb/" + cb[ci.index].key;
  }

  if (strcmp(topic, currentTopic.c_str()) == 0) {
    updateScreen = true;
  }

}
