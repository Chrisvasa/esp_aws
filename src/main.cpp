#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "DHT.h" // Include DHT library

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC "/telemetry"
#define AWS_IOT_SUBSCRIBE_TOPIC "/downlink"

#define DHTPIN 4          // Pin where the DHT sensor is connected
#define DHTTYPE DHT11     // DHT 11 sensor type

long sendInterval = 10000;  // interval at which to send to AWS

String THINGNAME = "CC8DA26A65CC";

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(1024);

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

void messageHandler(String &topic, String &payload);
void updateSettings(JsonDocument settingsObj);

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // get the macAddress
  THINGNAME = WiFi.macAddress();
  // remove colons from the MAC address string
  for (int i = 0; i < THINGNAME.length(); i++) {
    if (THINGNAME.charAt(i) == ':') {
      THINGNAME.remove(i, 1);
      i--;
    }
  }

  Serial.println();
  Serial.print("MAC Address: ");
  Serial.println(THINGNAME);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME.c_str())) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(THINGNAME + AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void setupShadow() {
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/get/accepted");
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/get/rejected");
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/update/delta");

  client.publish("$aws/things/" + THINGNAME + "/shadow/get");
}

bool publishTelemetry(String payload) {
  Serial.print("Publishing: ");
  Serial.println(payload);
  return client.publish(THINGNAME + AWS_IOT_PUBLISH_TOPIC, payload);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  JsonDocument doc;
  deserializeJson(doc, payload);

  // handle device shadow
  if (topic.endsWith("/shadow/get/accepted")) {
    updateSettings(doc["state"]["desired"]);
  } else if (topic.endsWith("/shadow/update/delta")) {
    updateSettings(doc["state"]);
  }
}

void updateSettings(JsonDocument settingsObj) {
  sendInterval = settingsObj["sendIntervalSeconds"].as<int>() * 1000;

  JsonDocument docResponse;
  docResponse["state"]["reported"] = settingsObj;
  char jsonBuffer[512];
  serializeJson(docResponse, jsonBuffer);

  // report back to device shadow
  Serial.print("Sending reported state to AWS: ");
  serializeJson(docResponse, Serial);

  client.publish("$aws/things/" + THINGNAME + "/shadow/update", jsonBuffer);
}

void setup() {
  Serial.begin(115200);
  dht.begin(); // Initialize the DHT sensor
  delay(2000);
  connectAWS();
  setupShadow();
}

void loop() {
  static unsigned long previousMillis = -sendInterval;

  client.loop();

  if (millis() - previousMillis >= sendInterval) {
    previousMillis = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Check if the readings are valid
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Construct the telemetry payload
    String payload = "{\"temperature\":" + String(temperature, 1) +
                     ",\"humidity\":" + String(humidity, 1) + "}";
    bool sendResult = publishTelemetry(payload);

    // Restart if send failed
    if (!sendResult) {
      ESP.restart();
    }
  }
}
