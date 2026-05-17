// Sender - Leitura de DHT22 e MH-Z19C e envio via LoRa

#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT22.h>
#include <HardwareSerial.h>

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

// Pinos dos Sensores
#define DHT_PIN 4
#define MHZ_RX_PIN 5 // RX para o MH-Z19C
#define MHZ_TX_PIN 6 // TX para o MH-Z19C
#define BAUDRATE 9600

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
SX1262 radio = new Module(SS_LORA, DIO1_LORA, RST_LORA, BUSY_LORA);
DHT22 dhtSensor(DHT_PIN);
HardwareSerial co2Serial(1); // Usando UART 1 para o sensor de CO2

void setup()
{
    Serial.begin(115200);

    // Inicializa a conexão UART para o sensor de CO2
    co2Serial.begin(BAUDRATE, SERIAL_8N1, MHZ_RX_PIN, MHZ_TX_PIN);

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

    Serial.println("Inicializando... Aguardando sensores.");
    delay(2000);
}

void loop()
{
    // 1. Leitura de Temperatura e Umidade (DHT22)
    float temp = dhtSensor.getTemperature();
    float hum = dhtSensor.getHumidity();
    if (isnan(temp) || isnan(hum))
    {
        Serial.println("Falha na leitura do sensor DHT22!");
    }

    // 2. Leitura de CO2 (MH-Z19C)
    int co2 = -1; // Valor inválido padrão
    uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

    // Limpa o buffer de entrada
    while (co2Serial.available() > 0)
        co2Serial.read();

    // Requisita leitura
    co2Serial.write(cmd, 9);
    co2Serial.flush();

    // Espera a resposta
    unsigned long startTime = millis();
    while (co2Serial.available() < 9 && (millis() - startTime) < 1000)
    {
        delay(10);
    }

    if (co2Serial.available() >= 9)
    {
        uint8_t response[9];
        co2Serial.readBytes(response, 9);

        // Valida o checksum
        uint8_t checksum = 0;
        for (int i = 1; i < 8; i++)
        {
            checksum += response[i];
        }
        checksum = 255 - checksum + 1;

        if (response[0] == 0xFF && response[1] == 0x86 && response[8] == checksum)
        {
            co2 = (response[2] * 256) + response[3];
        }
        else
        {
            Serial.println("Falha de Checksum no sensor de CO2.");
        }
    }
    else
    {
        Serial.println("Falha ao ler o CO2. Sem resposta.");
    }

    // 3. Monta a string para envio
    char payload[64];
    snprintf(payload, sizeof(payload), "t:%.1f,h:%.1f,co2:%d", temp, hum, co2);

    Serial.print("Transmitindo: ");
    Serial.println(payload);

    // 4. Envia por LoRa
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

    // 5. Atualiza o Display OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SENDER V3");

    display.setCursor(0, 12);
    display.print("Temp: ");
    if (!isnan(temp))
    {
        display.print(temp);
        display.print(" C");
    }
    else
    {
        display.print("ERRO");
    }

    display.setCursor(0, 24);
    display.print("Umid: ");
    if (!isnan(hum))
    {
        display.print(hum);
        display.print(" %");
    }
    else
    {
        display.print("ERRO");
    }

    display.setCursor(0, 36);
    display.print("CO2:  ");
    if (co2 != -1)
    {
        display.print(co2);
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

    // 6. Aguarda antes da proxima leitura
    delay(2000);
}
