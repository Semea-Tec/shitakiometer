// Teste da comunicacao do sensor DHT22

#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT22.h>
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

typedef struct struct_message {
    uint8_t mac[6];
    float temp;
} struct_message;

struct_message incomingReadings;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
SX1262 radio = new Module(SS_LORA, DIO1_LORA, RST_LORA, BUSY_LORA);

void OnDataRecv(const uint8_t * mac, const struct_message *incomingData, int len) {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.print("Mensagem recebida: ");
    Serial.println(incomingReadings.temp);
    Serial.printf("Mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
        incomingReadings.mac[0], incomingReadings.mac[1], incomingReadings.mac[2], incomingReadings.mac[3], incomingReadings.mac[4], incomingReadings.mac[5]);

    int state = radio.transmit(String(incomingData->temp).c_str());

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
    }
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

    // Seed the random number generator
    randomSeed(analogRead(1));

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
    display.println("SENDER V3 READY");
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

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao iniciar ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

    esp_err_t ret = esp_read_mac(incomingReadings.mac, ESP_MAC_WIFI_STA);

    if (ret == ESP_OK) {
        Serial.printf("MAC capturado com sucesso: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    incomingReadings.mac[0], incomingReadings.mac[1], incomingReadings.mac[2], incomingReadings.mac[3], incomingReadings.mac[4], incomingReadings.mac[5]);
    } else {
        Serial.println("Erro crítico: Não foi possível ler o MAC da memória.");
    }
}

void loop()
{
    // Update Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SENDER V3");
    display.setCursor(0, 20);
    display.print("Sending Temperature: ");
    display.setTextSize(2);
    display.setCursor(0, 35);
    display.println(incomingReadings.temp);
    display.display();

    delay(2000);
}
