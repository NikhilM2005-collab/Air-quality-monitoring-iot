// Air Quality Monitoring System with ESP32 + MQ-135 + DHT22 -> ThingSpeak
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 4        // DHT22 data pin (GPIO4)
#define DHTTYPE DHT22
#define MQ_PIN 34       // MQ-135 analog output to ADC1 (GPIO34)

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* thingspeakKey = "YOUR_API_KEY"; // Replace with your ThingSpeak API key

String thingspeakUrl = "http://api.thingspeak.com/update";

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MQ_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

float readMQ135() {
  int raw = analogRead(MQ_PIN);
  float v = raw / 4095.0;
  float voltage = v * 3.3;
  float proxy = (voltage / 3.3) * 500.0; // approximate scaling
  return proxy;
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  float mq = readMQ135();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("DHT read error!");
    return;
  }

  Serial.printf("MQ: %.2f | Temp: %.2f C | Hum: %.2f %%\n", mq, temp, hum);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = thingspeakUrl + "?api_key=" + thingspeakKey +
                 "&field1=" + String(mq, 2) +
                 "&field2=" + String(temp, 2) +
                 "&field3=" + String(hum, 2);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent to ThingSpeak");
    } else {
      Serial.println("Error sending data");
    }
    http.end();
  }

  delay(60000); // Update every 60 seconds
}
