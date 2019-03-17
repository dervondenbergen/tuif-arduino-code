extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "ESPRotary.h";
#include "Button2.h";
#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>

/////////////////////////////////////////////////////////////////

typedef struct Departures {
  String key;
  int time0;
  int time1;
};

typedef struct WL {
  String line;
  int activeDirection;
  int keys[2];
  String direction[2];
  String name[2];
};

typedef struct CB {
  String key;
  String name;
};

typedef enum ItemType {
  wienerlinien,
  citybike
};

typedef struct Items {
  ItemType type;
  int index;
};

Departures departures[7];
int departurecount;

Items items[4];
int itemcount = 0;

WL wl[3];
int wlcount = 0;

CB cb[1];
int cbcount = 0;

int currentItem = 0;

/////////////////////////////////////////////////////////////////

#include "config.h";

void loadDataConfig () {
  // Größe entsteht so:   Anzahl an Linien   + 1 * CB Objekt       + 3 * WL Objekt         + 3 * 3 rbl/name/richtung + extra platz.
  const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(6) + 9*JSON_ARRAY_SIZE(2)  + 360;
  DynamicJsonDocument jsonitems(capacity);
  const char* json = "[{\"t\":\"wl\",\"l\":\"49\",\"a\":0,\"k\":[1487,1486],\"d\":[\"Ring/Volkstheater U\",\"Hütteldorf, Bujattigasse\"],\"n\":[\"Siebensterngasse\",\"Siebensterngasse\"]},{\"t\":\"wl\",\"l\":\"13A\",\"a\":1,\"k\":[704,688],\"d\":[\"Skodagasse\",\"Hauptbahnhof\"],\"n\":[\"Siebensterngasse\",\"Neubaugasse\"]},{\"t\":\"wl\",\"l\":\"U3\",\"a\":0,\"k\":[4918,4923],\"d\":[\"Ottakring\",\"Simmering\"],\"n\":[\"Neubaugasse\",\"Neubaugasse\"]},{\"t\":\"cb\",\"k\":\"30865b5a6af188b6560f668c3bc800d4\",\"n\":\"Siebensternplatz\"}]";

  deserializeJson(jsonitems, json);
  JsonArray itemlist = jsonitems.as<JsonArray>();

  int dc = 0;
  for(JsonObject item : itemlist) {

    const char* type = item["t"];

    if (strcmp(type, "wl") == 0) {
      items[itemcount].type = wienerlinien;
      items[itemcount].index = wlcount;
      
      const char* line = item["l"];
      int activeDirection = item["a"];
      int k0 = item["k"][0];
      int k1 = item["k"][1];
      const char* d0 = item["d"][0];
      const char* d1 = item["d"][1];
      const char* n0 = item["n"][0];
      const char* n1 = item["n"][1];
      

      wl[wlcount].line = String(line);
      wl[wlcount].activeDirection = activeDirection;
      wl[wlcount].keys[0] = k0;
      wl[wlcount].keys[1] = k1;
      wl[wlcount].direction[0] = String(d0);
      wl[wlcount].direction[1] = String(d1);
      wl[wlcount].name[0] = String(n0);
      wl[wlcount].name[1] = String(n1);

      String dk0 = String("wl/");
      dk0.concat(k0);
      dk0.concat("/");
      dk0.concat(line);
      departures[dc].key = dk0;
      dc++;
      String dk1 = String("wl/");
      dk1.concat(k1);
      dk1.concat("/");
      dk1.concat(line);
      departures[dc].key = dk1;
      dc++;

      wlcount++;
    }

    if (strcmp(type, "cb") == 0) {
      items[itemcount].type = citybike;
      items[itemcount].index = cbcount;

      const char* key = item["k"];
      const char* name = item["n"];
      
      cb[cbcount].key = String(key);
      cb[cbcount].name = String(name);

      String datakey = String("cb/");
      datakey.concat(key);
      departures[dc].key = datakey;
      dc++;
      
      cbcount++;
    };

    itemcount++;
    
  }

  departurecount = dc;
}

/////////////////////////////////////////////////////////////////

#define ENABLE_GxEPD2_GFX 0

GxEPD2_3C<GxEPD2_213c, GxEPD2_213c::HEIGHT> display(GxEPD2_213c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

U8G2_FOR_ADAFRUIT_GFX u8g2;

void setupDisplay () {
  
  display.init();
  display.setRotation(3);
  u8g2.begin(display);
  display.setFullWindow();
  
  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
  u8g2.setForegroundColor(GxEPD_BLACK);
  u8g2.setBackgroundColor(GxEPD_WHITE);
  
}

bool updateScreen = true;

/////////////////////////////////////////////////////////////////

#include "Visualizer.h";

/////////////////////////////////////////////////////////////////

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

#include "Mqtt.h";

void setupMqtt () {
    
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT); 
  mqttClient.setClientId(MQTT_CLIENT_ID);

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
}

/////////////////////////////////////////////////////////////////

TimerHandle_t wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void setupWiFi () {
  WiFi.onEvent(WiFiEvent);
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  connectToWifi();
}

/////////////////////////////////////////////////////////////////

#define ROTARY_PIN1  39
#define ROTARY_PIN2 35
#define ROTARY_MOVES 4

#define BUTTON_PIN  37

ESPRotary rotaryEncoder = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, ROTARY_MOVES);
Button2 rotaryButton = Button2(BUTTON_PIN);

int rotary_step = 0;

#include "Interact.h";

void setupEncoder () {
  rotaryEncoder.setChangedHandler(changeItem);
  rotaryButton.setPressedHandler(switchDirection);
}

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  loadDataConfig();

  setupMqtt();
  
  setupWiFi();

  setupDisplay();

  setupEncoder();
  
}

void loop() {
  if (updateScreen) {
    showState();
    updateScreen = false;
  }
  rotaryEncoder.loop();
  rotaryButton.loop();
}
