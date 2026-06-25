// Sensor (Camada de Coleta) - Seeed Studio XIAO ESP32-C5
//
// Le DHT22 (temperatura/umidade) e MH-Z19C (CO2) e envia os dados via
// ESP-NOW (broadcast, 2.4GHz) para o concentrador Heltec, que retransmite
// via LoRa. Ver Tests/Network/sender/concentradorHeltec.ino.
//
// Board: "XIAO_ESP32C5" (pacote "esp32" by Espressif Systems, v3.3.5+)
// Pinagem: ver Docs/CODEBASE_MEMORY.md (secao XIAO ESP32-C5)

#include <esp_now.h>
#include <esp_mac.h>
#include <WiFi.h>
#include <DHT22.h>
#include <HardwareSerial.h>

// Pinos do XIAO ESP32-C5 (board com poucos GPIOs, ver datasheet Seeed)
#define DHT_PIN 25     // D2 - GPIO25, livre, sem funcao alternativa
#define MHZ_RX_PIN 12  // D7 - recebe o TX do sensor MH-Z19C
#define MHZ_TX_PIN 11  // D6 - envia para o RX do sensor MH-Z19C
#define STATUS_LED 27  // LED amarelo onboard do XIAO ESP32-C5
#define BAUDRATE 9600

// Endereco de broadcast: dispensa descobrir/fixar o MAC do concentrador.
// Qualquer Heltec dentro do alcance com o sketch concentrador roda e recebe.
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message
{
    float temp;
    float hum;
    int co2;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;
DHT22 dhtSensor(DHT_PIN);
HardwareSerial co2Serial(1); // UART1 para o sensor de CO2

void setup()
{
    Serial.begin(115200);

    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    co2Serial.begin(BAUDRATE, SERIAL_8N1, MHZ_RX_PIN, MHZ_TX_PIN);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Erro ao iniciar ESP-NOW");
        return;
    }

    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Falha ao adicionar peer de broadcast");
        return;
    }

    Serial.println("Sensor XIAO ESP32-C5 pronto. Aguardando leituras...");
    delay(2000);
}

// Le o CO2 do MH-Z19C via comando UART (protocolo padrao, mesmo usado
// no concentrador antigo Tests/Network/sender/senderWithSensors.ino).
int readCO2()
{
    uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

    while (co2Serial.available() > 0)
        co2Serial.read();

    co2Serial.write(cmd, 9);
    co2Serial.flush();

    unsigned long startTime = millis();
    while (co2Serial.available() < 9 && (millis() - startTime) < 1000)
    {
        delay(10);
    }

    if (co2Serial.available() < 9)
    {
        Serial.println("Falha ao ler o CO2. Sem resposta.");
        return -1;
    }

    uint8_t response[9];
    co2Serial.readBytes(response, 9);

    uint8_t checksum = 0;
    for (int i = 1; i < 8; i++)
        checksum += response[i];
    checksum = 255 - checksum + 1;

    if (response[0] != 0xFF || response[1] != 0x86 || response[8] != checksum)
    {
        Serial.println("Falha de Checksum no sensor de CO2.");
        return -1;
    }

    return (response[2] * 256) + response[3];
}

void loop()
{
    float temp = dhtSensor.getTemperature();
    float hum = dhtSensor.getHumidity();
    if (isnan(temp) || isnan(hum))
    {
        Serial.println("Falha na leitura do sensor DHT22!");
    }

    int co2 = readCO2();

    myData.temp = temp;
    myData.hum = hum;
    myData.co2 = co2;

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK)
    {
        digitalWrite(STATUS_LED, HIGH);
        Serial.printf("Enviado via ESP-NOW: t:%.1f,h:%.1f,co2:%d\n", myData.temp, myData.hum, myData.co2);
    }
    else
    {
        digitalWrite(STATUS_LED, LOW);
        Serial.println("Erro ao enviar via ESP-NOW");
    }

    delay(2000);
}
