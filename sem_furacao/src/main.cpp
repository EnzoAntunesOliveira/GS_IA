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

#define DEVICE_ID             "sem_furacao"

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
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nWiFi OK");
  net.setInsecure();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Conectando MQTT...");
    if (mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS)) {
      Serial.println(" conectado!");
    } else {
      Serial.printf(" falhou, rc=%d. Tentando em 5s\n", mqtt.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastPublish < PUBLISH_INTERVAL) return;
  lastPublish = now;

  // ─────────── Faixas para SEM FURACÃO ───────────
  float temperature = randomFloat(20.0, 28.0);  
  float humidity    = randomFloat(50.0, 70.0);  
  int   pressure    = random(980, 1020);        

  // ─────────── Monta JSON ───────────
  StaticJsonDocument<200> doc;
  doc["deviceId"]    = DEVICE_ID;
  doc["timestamp"]   = now;
  doc["temperature"] = temperature;
  doc["humidity"]    = humidity;
  doc["pressure"]    = pressure;

  char payload[256];
  serializeJson(doc, payload);

  // ─────────── Publica no tópico hurricane/sem_furacao/data ───────────
  String topic = String("hurricane/") + DEVICE_ID + "/data";
  mqtt.publish(topic.c_str(), payload);

  Serial.printf("Sem Furacao → %s\n", payload);

  // ─────────── Envia ao ThingSpeak ───────────
  HTTPClient http;
  String url = String("http://api.thingspeak.com/update?api_key=")
               + THINGSPEAK_API_KEY
               + "&field1=" + String(temperature)
               + "&field2=" + String(humidity)
               + "&field3=" + String(pressure)
               + "&field4=1";  
  http.begin(url);
  int httpCode = http.GET();
  Serial.printf("ThingSpeak HTTP code: %d\n", httpCode);
  http.end();
}
