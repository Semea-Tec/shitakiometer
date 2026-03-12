#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Pins for Heltec LoRa V3 (ESP32-S3)
#define SCK_LORA 9
#define MISO_LORA 11
#define MOSI_LORA 10
#define SS_LORA 8
#define RST_LORA 12
#define BUSY_LORA 13
#define DIO1_LORA 14

#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21
#define VEXT_PIN 36 // GPIO 36 controls Vext on V3
#define LED_PIN 35  // GPIO 35 is the white LED

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BAND 915.0 // MHz

// WiFi and MQTT Configuration
const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "192.168.15.82";
const int mqtt_port = 1883;
const char* mqtt_topic = "sensores/temperatura";

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
SX1262 radio = new Module(SS_LORA, DIO1_LORA, RST_LORA, BUSY_LORA);

// Flag para indicar que um pacote foi recebido via interrupção
volatile bool receivedFlag = false;

// Função de interrupção (ISR)
void setFlag(void) {
    receivedFlag = true;
}

void setup_wifi() {
    delay(10);
    Serial.println();

    // Garante que o WiFi inicie em um estado limpo (Station Mode)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.println(")");
            delay(10);
        }
    }
    Serial.println("");

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi desconectado. Reconectando...");
            WiFi.disconnect();
            WiFi.begin(ssid, password);
            delay(5000);
            continue; // Não tenta conectar no MQTT se não tiver WiFi
        }

        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
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

void setup()
{
    Serial.begin(115200);

    // Turn on Vext power (V3 uses GPIO 36)
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, LOW);
    delay(100);

    // Turn on built-in LED to show it's alive
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // Manual OLED Reset
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    // Initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        for (;;)
            ;
    }

    display.clearDisplay();
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0xFF); // Max brightness
    display.dim(false);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("RECEIVER V3 READY");
    display.display();

    // Setup WiFi and MQTT
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);

    // Initialize LoRa
    Serial.print(F("[LoRa] Initializing ... "));
    int state = radio.begin(BAND);
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }

    // Configura a interrupção para quando receber um pacote
    radio.setDio1Action(setFlag);

    // Começa a escutar (modo não bloqueante)
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("[LoRa] Listening for packets..."));
    } else {
        Serial.print(F("[LoRa] Failed to start receive, code "));
        Serial.println(state);
    }
}

void loop()
{
    if (!client.connected()) {
        reconnect();
        // Reinicia a escuta LoRa após reconectar, por garantia
        radio.startReceive();
    }
    client.loop();

    // Verifica se a flag de interrupção foi ativada
    if (receivedFlag) {
        // Reseta a flag
        receivedFlag = false;

        String str;
        // Lê os dados recebidos
        int state = radio.readData(str);

        if (state == RADIOLIB_ERR_NONE)
        {
        Serial.println(F("[LoRa] Data received!"));
        Serial.print(F("[LoRa] Data:\t\t"));
        Serial.println(str);

        // Publish to MQTT
        if (client.publish(mqtt_topic, str.c_str())) {
            Serial.println(F("[MQTT] Data published successfully"));
        } else {
            Serial.println(F("[MQTT] Failed to publish data"));
        }

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("RECEIVER V3");

        display.setCursor(0, 15);
        display.println("Value Received:");

        display.setTextSize(2);
        display.setCursor(0, 30);
        display.println(str);

        display.setTextSize(1);
        display.setCursor(0, 50);
        display.print("RSSI: ");
        display.print(radio.getRSSI());
        display.print(" dBm");
        display.display();
    }
    
    // Volta a escutar novos pacotes
    radio.startReceive();
    }
}
