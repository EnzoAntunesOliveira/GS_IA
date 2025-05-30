#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// ─────────── Configurações gerais ───────────
#define WIFI_SSID               "Wokwi-GUEST"
#define WIFI_PASS               ""
#define MQTT_SERVER             "3e1726090cad44fd974b0960714f0f50.s1.eu.hivemq.cloud"
#define MQTT_PORT               8883
#define MQTT_USER               "EnzoA"
#define MQTT_PASS               "Enzo1ia."
#define THINGSPEAK_API_KEY      "9QJ8QR2D5KJ2GI80"
#define THINGSPEAK_CHANNEL_ID   2975134
#define PUBLISH_INTERVAL        10000  

WiFiClientSecure net;
PubSubClient    mqtt(net);
unsigned long   lastPublish = 0;

// ─────────── Gera um float aleatório ───────────
float randomFloat(float minVal, float maxVal) {
  return minVal + (random(0, 10001) / 10000.0f) * (maxVal - minVal);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Iniciando...");

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
    if (mqtt.connect("esp32_aleatorio", MQTT_USER, MQTT_PASS)) {
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

  // ─────────── Sorteia o tipo de sensor ───────────
  int estado = random(0, 3);
  const char* DEVICE_ID;
  float tempMin, tempMax, humMin, humMax;
  int   presMin, presMax;

  switch (estado) {
    case 2:
      DEVICE_ID = "furacao";
      tempMin = 30.0; tempMax = 40.0;
      humMin  = 90.0; humMax  = 100.0;
      presMin = 900;  presMax = 960;
      break;
    case 1:
      DEVICE_ID = "possivel_furacao";
      tempMin = 26.0; tempMax = 32.0;
      humMin  = 80.0; humMax  = 90.0;
      presMin = 960;  presMax = 980;
      break;
    default:
      DEVICE_ID = "sem_furacao";
      tempMin = 20.0; tempMax = 28.0;
      humMin  = 50.0; humMax  = 70.0;
      presMin = 980;  presMax = 1020;
      break;
  }

  // ─────────── Gera valores simulados ───────────
  float temperature = randomFloat(tempMin, tempMax);
  float humidity    = randomFloat(humMin, humMax);
  int   pressure    = random(presMin, presMax + 1);

  // ─────────── Monta JSON ───────────
  StaticJsonDocument<256> doc;
  doc["deviceId"]    = DEVICE_ID;
  doc["timestamp"]   = now;
  doc["temperature"] = temperature;
  doc["humidity"]    = humidity;
  doc["pressure"]    = pressure;

  char payload[256];
  serializeJson(doc, payload);

  // ─────────── Publica no tópico hurricane/{DEVICE_ID}/data ───────────
  String topic = String("hurricane/") + DEVICE_ID + "/data";
  mqtt.publish(topic.c_str(), payload);

  Serial.printf("Publicado em %s: %s\n", topic.c_str(), payload);

  // ─────────── Envia ao ThingSpeak ───────────
  HTTPClient http;
  String url = String("http://api.thingspeak.com/update?api_key=") 
               + THINGSPEAK_API_KEY
               + "&field1=" + String(temperature)
               + "&field2=" + String(humidity)
               + "&field3=" + String(pressure)
               + "&field4=" + String(estado + 1);  /
  http.begin(url);
  int httpCode = http.GET();
  Serial.printf("ThingSpeak HTTP code: %d\n", httpCode);
  http.end();
}
