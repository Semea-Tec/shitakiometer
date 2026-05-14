#include <WiFi.h>
#include <PubSubClient.h>

// Access Point (AP) Configuration
const char* ssid = "ESP32_Shitakiometer";
const char* password = "password123";

// MQTT Broker Configuration
const char* mqtt_server = "192.168.4.2";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
float temp = 25.0;
float hum = 60.0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Setting up Access Point: ");
  Serial.println(ssid);

  // Set ESP32 as an Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("");
  Serial.println("Access Point started!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Waiting for server to connect to this Wi-Fi network...");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Mock-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  // Send data every 5 seconds
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Simulate a slight variation in the values
    temp += random(-5, 6) / 10.0;
    hum += random(-10, 11) / 10.0;

    // Limit values to avoid absurd readings
    if (temp < 15.0) temp = 15.0;
    if (temp > 35.0) temp = 35.0;
    if (hum < 30.0) hum = 30.0;
    if (hum > 90.0) hum = 90.0;

    String tempStr = String(temp, 2);
    String humStr = String(hum, 2);

    Serial.print("Publishing -> Temperatura: ");
    Serial.print(tempStr);
    Serial.print(" °C | Umidade: ");
    Serial.print(humStr);
    Serial.println(" %");

    client.publish("sensores/temperatura", tempStr.c_str());
    client.publish("sensores/umidade", humStr.c_str());
  }
}
