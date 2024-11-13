#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi settings
const char *ssid = "Wokwi-GUEST"; // Replace with your WiFi name
const char *password = "";        // Replace with your WiFi password

// MQTT Broker settings
const char *mqtt_broker = "broker.emqx.io";    // EMQX broker endpoint
const char *mqtt_topic = "UTS_IOT/emqx/esp32"; // MQTT topic
const char *mqtt_username = "ichlasul";        // MQTT username for authentication
const char *mqtt_password = "ichlasul07";      // MQTT password for authentication
const int mqtt_port = 1883;                    // MQTT port (TCP)

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// DHT Sensor settings
#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// GPIO pin assignments
const int LED_GREEN = 5;
const int LED_YELLOW = 10;
const int LED_RED = 12;
const int RELAY_PIN = 7;
const int BUZZER_PIN = 9;

void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup()
{
  Serial.begin(115200);

  // Initialize pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize DHT sensor
  dht.begin();

  connectToWiFi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  connectToMQTTBroker();
}

void connectToWiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker()
{
  while (!mqtt_client.connected())
  {
    String client_id = "esp8266-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Connected to MQTT broker");
      mqtt_client.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  // Process incoming MQTT messages if needed
}

void loop()
{
  mqtt_client.loop();

  // Read temperature and humidity from DHT sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if reading failed
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Logic for LED, buzzer, and relay control based on temperature
  if (temperature > 35)
  {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
  }
  else if (temperature >= 30 && temperature <= 35)
  {
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
  }
  else
  {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
  }

  // Control the pump (relay) based on temperature
  if (temperature > 30)
  {
    digitalWrite(RELAY_PIN, HIGH); // Turn on the pump
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW); // Turn off the pump
  }

  // Send data to MQTT broker in JSON format
  String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"pump_status\":" + (digitalRead(RELAY_PIN) ? "ON" : "OFF") + "}";
  if (!mqtt_client.publish(mqtt_topic, payload.c_str()))
  {
    Serial.println("Failed to publish message to MQTT topic");
  }

  // Print sensor data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(2000); // Delay between readings to avoid flooding the broker
}
