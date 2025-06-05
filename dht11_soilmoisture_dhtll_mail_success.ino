#include <WiFi.h>             // ESP32 WiFi Library
#include <WiFiClientSecure.h> // Secure client for TLS
#include <PubSubClient.h>     // MQTT Client
#include <DHT.h>              // DHT11 Sensor Library

// WiFi Credentials
const char* ssid = "Redmi 9 Power";         // 🔹 Replace with your WiFi SSID
const char* password = "12341234";          // 🔹 Replace with your WiFi Password

// HiveMQ Cloud Credentials
const char* mqtt_server = "0a5128acc6e34a18bfe48618245ec1a2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Secure TLS MQTT Port
const char* mqtt_user = "GRahul";   // 🔹 Replace with your HiveMQ username
const char* mqtt_password = "Rahul123"; // 🔹 Replace with your HiveMQ password
const char* topic = "madhuriot"; // 🔹 MQTT topic

// DHT11 Sensor Setup
#define DHTPIN 4         // GPIO pin where DHT11 is connected
#define DHTTYPE DHT11    // Define sensor type (DHT11)
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture Sensor Setup
#define SOIL_MOISTURE_A0 34  // Analog output from sensor (A0 → GPIO34)
#define SOIL_MOISTURE_D0 32  // Digital output from sensor (D0 → GPIO32)

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
    if (client.connect("ESP32_Client", mqtt_user, mqtt_password)) {
      Serial.println("\n✅ Connected to HiveMQ!");
    } else {
      Serial.print("❌ MQTT Connection Failed! Error Code: ");
      Serial.println(client.state());  // Print error code
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();  // Initialize DHT11 sensor

  pinMode(SOIL_MOISTURE_D0, INPUT); // Digital input from soil sensor

  connectToWiFi();
  
  // Set up TLS connection (Skip SSL Certificate Verification)
  espClient.setInsecure();

  // Connect to HiveMQ
  client.setServer(mqtt_server, mqtt_port);
  connectToMQTT();
}
void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // Read Temperature & Humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read Soil Moisture Data
  int soilMoistureAnalog = analogRead(SOIL_MOISTURE_A0);  // Get Analog moisture value (0-4095)
  int soilMoistureDigital = digitalRead(SOIL_MOISTURE_D0); // Get Digital moisture status (1 = Dry, 0 = Wet)

  // Check if the readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("⚠️ Failed to read from DHT sensor!");
    return;
  }

  // Create JSON payload
  char payload[150];
  snprintf(payload, sizeof(payload), 
           "{\"temperature\": %.2f, \"humidity\": %.2f, \"soil_moisture\": %d, \"soil_status\": \"%s\"}", 
           temperature, humidity, soilMoistureAnalog, 
           soilMoistureDigital == 1 ? "Dry" : "Wet");

  // Publish Data
  Serial.print("📤 Publishing Data: ");
  Serial.println(payload);
  client.publish(topic, payload);
  
  delay(5000); // Wait 5 seconds before publishing again
}
