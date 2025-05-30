#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define WIFI_SSID             "Wokwi-GUEST"
#define WIFI_PASS             ""
#define MQTT_SERVER           "3e1726090cad44fd974b0960714f0f50.s1.eu.hivemq.cloud"
#define MQTT_PORT             8883
#define MQTT_USER             "EnzoA"
#define MQTT_PASS             "Enzo1ia."
#define THINGSPEAK_API_KEY    "9QJ8QR2D5KJ2GI80"
#define THINGSPEAK_CHANNEL_ID 2975134
#define PUBLISH_INTERVAL      10000

#define DEVICE_ID             "furacao"

WiFiClientSecure net;
PubSubClient    mqtt(net);
unsigned long   lastPublish = 0;

float randomFloat(float minVal, float maxVal) {
  return minVal + (random(0, 10001) / 10000.0f) * (maxVal - minVal);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  net.setInsecure();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
}

void reconnect() {
  while (!mqtt.connected()) {
    if (mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS)) break;
    delay(5000);
  }
}

void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastPublish < PUBLISH_INTERVAL) return;
  lastPublish = now;

  // ─────────── Faixas para FURACÃO ───────────
  float temperature = randomFloat(30.0, 40.0);
  float humidity    = randomFloat(90.0, 100.0);
  int   pressure    = random(900, 960);

  // ─────────── Monta JSON ───────────
  StaticJsonDocument<200> doc;
  doc["deviceId"]    = DEVICE_ID;
  doc["timestamp"]   = now;
  doc["temperature"] = temperature;
  doc["humidity"]    = humidity;
  doc["pressure"]    = pressure;

  char payload[256];
  serializeJson(doc, payload);

  // ─────────── Publica no tópico hurricane/furacao/data ───────────
  String topic = String("hurricane/") + DEVICE_ID + "/data";
  mqtt.publish(topic.c_str(), payload);

  Serial.printf("Furacao → %s\n", payload);

  // ─────────── Envia ao ThingSpeak ───────────
  HTTPClient http;
  String url = String("http://api.thingspeak.com/update?api_key=")
               + THINGSPEAK_API_KEY
               + "&field1=" + String(temperature)
               + "&field2=" + String(humidity)
               + "&field3=" + String(pressure)
               + "&field4=3";  
  http.begin(url);
  int httpCode = http.GET();
  Serial.printf("ThingSpeak HTTP code: %d\n", httpCode);
  http.end();
}
