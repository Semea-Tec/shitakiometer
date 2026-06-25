// Concentrador (Camada de Borda) - Heltec WiFi LoRa 32 V3 (ESP32-S3)
//
// Recebe os dados do sensor (Seeed XIAO ESP32-C5, ESP-NOW broadcast,
// ver Tests/Network/sensor/sensor.ino) e retransmite via LoRa P2P (915MHz)
// para o receptor/gateway proximo ao usuario final. Mantem o mesmo formato
// de payload usado anteriormente ("t:..,h:..,co2:.."), entao os sketches
// de receiver (receiver_webServer.ino / receiver_mqtt.ino) nao precisam
// de alteracao.
//
// Substitui o papel de leitura de sensores que existia em
// senderWithSensors.ino (agora mantido apenas como referencia legada,
// modo standalone sem o XIAO).

#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>
#include <WiFi.h>

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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
SX1262 radio = new Module(SS_LORA, DIO1_LORA, RST_LORA, BUSY_LORA);

// Mesma struct enviada pelo sensor XIAO ESP32-C5 (Tests/Network/sensor/sensor.ino)
typedef struct struct_message
{
    float temp;
    float hum;
    int co2;
} struct_message;

volatile struct_message lastReading = {NAN, NAN, -1};
volatile bool hasNewReading = false;

void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
    if (len != sizeof(struct_message))
        return;
    memcpy((void *)&lastReading, incomingData, sizeof(struct_message));
    hasNewReading = true;
}

void setup()
{
    Serial.begin(115200);

    // Turn on Vext power for OLED and LoRa (V3 uses GPIO 36)
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
        Serial.println("OLED Failed");
        for (;;)
            ;
    }

    display.clearDisplay();
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0xFF); // Max brightness
    display.dim(false);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("CONCENTRADOR V3");
    display.println("Aguardando ESP-NOW");
    display.display();

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

    // Initialize ESP-NOW (recebe do sensor XIAO ESP32-C5)
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Erro ao iniciar ESP-NOW");
        while (true)
            ;
    }
    esp_now_register_recv_cb(onDataReceived);

    Serial.println("Concentrador pronto. Aguardando dados do sensor...");
    delay(1000);
}

void loop()
{
    if (!hasNewReading)
    {
        delay(50);
        return;
    }

    struct_message reading;
    memcpy(&reading, (void *)&lastReading, sizeof(struct_message));
    hasNewReading = false;

    // Monta a string para envio (mesmo formato consumido pelos receivers)
    char payload[64];
    snprintf(payload, sizeof(payload), "t:%.1f,h:%.1f,co2:%d", reading.temp, reading.hum, reading.co2);

    Serial.print("Transmitindo via LoRa: ");
    Serial.println(payload);

    int state = radio.transmit(payload);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("Envio LoRa com sucesso!"));
    }
    else
    {
        Serial.print(F("Falha LoRa, codigo "));
        Serial.println(state);
    }

    // Atualiza o Display OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("CONCENTRADOR V3");

    display.setCursor(0, 12);
    display.print("Temp: ");
    if (!isnan(reading.temp))
    {
        display.print(reading.temp);
        display.print(" C");
    }
    else
    {
        display.print("ERRO");
    }

    display.setCursor(0, 24);
    display.print("Umid: ");
    if (!isnan(reading.hum))
    {
        display.print(reading.hum);
        display.print(" %");
    }
    else
    {
        display.print("ERRO");
    }

    display.setCursor(0, 36);
    display.print("CO2:  ");
    if (reading.co2 != -1)
    {
        display.print(reading.co2);
        display.print(" ppm");
    }
    else
    {
        display.print("ERRO");
    }

    display.setCursor(0, 50);
    display.print("LoRa: ");
    display.println(state == RADIOLIB_ERR_NONE ? "OK" : "FALHA");

    display.display();
}
