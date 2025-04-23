#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "HX711.h"

// Wi-Fi Settings
const char* ssid = "LinkIMS5G";
const char* password = "IMS@i4studio";

// MQTT Broker Settings
const char* mqtt_server = "main.messaging.iot.scemvi.suite.maximo.com";
const char* mqttClient_id = "d:main:H_growth:H_growth_01";
const char* topic = "iot-2/evt/weight_monitoring/fmt/json";
const int mqtt_port = 443;
const char* mqtt_username = "use-token-auth";
const char* mqtt_password = ")0?RKxRT6_TZd+6R&B";

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 14;

// Calibration factor
float calibration_factor = 21000.0; 

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
HX711 scale;

// Global variables
bool isWiFiConnected = false;
unsigned long lastMeasurementTime = 0;
unsigned long measurementInterval = 10000; // 10 seconds

// Function Prototypes
void manageWiFi();
void manageMQTT();
float getAverageWeight(int samples);
void sendWeightOverMQTT();

// Manage Wi-Fi connection
void manageWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      isWiFiConnected = true;
      Serial.println("\nWi-Fi connected.");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      isWiFiConnected = false;
      Serial.println("\nWi-Fi connection failed.");
    }
  }
}

// Manage MQTT connection
void manageMQTT() {
  if (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    mqttClient.setServer(mqtt_server, mqtt_port);
    int retries = 3;
    while (!mqttClient.connected() && retries > 0) {
      if (mqttClient.connect(mqttClient_id, mqtt_username, mqtt_password)) {
        Serial.println("MQTT connected.");
      } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(mqttClient.state());
        retries--;
        delay(5000);
      }
    }
  }
}

// Get average weight using 1000 readings
float getAverageWeight(int samples) {
  float total = 0;
  for (int i = 0; i < samples; i++) {
    total += scale.get_units();
    delay(1);  // Small delay between readings
  }
  return total / samples;
}

// Send weight data over MQTT
void sendWeightOverMQTT() {
  if (isWiFiConnected && mqttClient.connected()) {
    float weight = getAverageWeight(100);  // Average of 1000 readings

    char mqtt_data[64];
    snprintf(mqtt_data, sizeof(mqtt_data), "{\"weight\": %.2f}", weight);

    if (mqttClient.publish(topic, mqtt_data)) {
      Serial.print("Weight data sent: ");
      Serial.println(mqtt_data);
    } else {
      Serial.println("Failed to send weight data.");
    }
  } else {
    Serial.println("MQTT or Wi-Fi not connected. Retrying...");
    manageMQTT();
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  Serial.println("Scale initialized.");

  // Set insecure SSL
  espClient.setInsecure();

  // Connect to Wi-Fi
  manageWiFi();

  // Connect to MQTT
  if (isWiFiConnected) {
    manageMQTT();
  }
}

void loop() {
  // Reconnect Wi-Fi if disconnected
  manageWiFi();

  // Reconnect MQTT if disconnected
  if (!mqttClient.connected()) {
    manageMQTT();
  }
  mqttClient.loop();

  // Send weight data at regular intervals
  if (millis() - lastMeasurementTime >= measurementInterval) {
    sendWeightOverMQTT();
    lastMeasurementTime = millis();
  }
}