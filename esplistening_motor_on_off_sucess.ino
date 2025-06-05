#include <WiFi.h>             // ESP32 WiFi Library
#include <WiFiClientSecure.h> // Secure client for TLS
#include <PubSubClient.h>     // MQTT Client
#include <ArduinoJson.h>      // JSON Parsing Library

// WiFi Credentials
const char* ssid = "Redmi 9 Power";         // 🔹 Replace with your WiFi SSID
const char* password = "12341234";          // 🔹 Replace with your WiFi Password

// HiveMQ Cloud Credentials
const char* mqtt_server = "0a5128acc6e34a18bfe48618245ec1a2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Secure TLS MQTT Port
const char* mqtt_user = "GRahul";   // 🔹 Replace with your HiveMQ username
const char* mqtt_password = "Rahul123"; // 🔹 Replace with your HiveMQ password
const char* topic = "motorControlTopic"; // 🔹 MQTT topic to listen to

// Motor/Relay Pin
#define MOTOR_PIN 25  // GPIO pin for the motor/relay

WiFiClientSecure espClient;
PubSubClient client(espClient);

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
}

void connectToMQTT() {
  Serial.print("Connecting to HiveMQ...");
  while (!client.connected()) {
    if (client.connect("ESP32_Motor_Controller", mqtt_user, mqtt_password)) {
      Serial.println("\n✅ Connected to HiveMQ!");
      client.subscribe(topic); // Subscribe to the motorControlTopic
    } else {
      Serial.print("❌ MQTT Connection Failed! Error Code: ");
      Serial.println(client.state());  // Print error code
      delay(2000);
    }
  }
}

// Callback function to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📥 Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  // Convert payload to a string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Parse the JSON message
  StaticJsonDocument<200> doc; // Adjust size as needed
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("⚠️ JSON Parsing Failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract motor_status value
  int motorStatus = doc["motor_status"]; // Get the motor_status value

  // Control the motor/relay based on the motor_status value
  if (motorStatus == 1) {
    Serial.println("🚀 Turning ON the motor/relay...");
    digitalWrite(MOTOR_PIN, HIGH); // Turn ON the motor/relay
  } else if (motorStatus == 0) {
    Serial.println("🛑 Turning OFF the motor/relay...");
    digitalWrite(MOTOR_PIN, LOW); // Turn OFF the motor/relay
  } else {
    Serial.println("⚠️ Invalid motor_status value! Expected 0 or 1.");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize Motor/Relay pin
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW); // Ensure motor/relay is off initially

  connectToWiFi();
  
  // Set up TLS connection (Skip SSL Certificate Verification)
  espClient.setInsecure();

  // Connect to HiveMQ
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Set the callback function
  connectToMQTT();
}

void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();
}