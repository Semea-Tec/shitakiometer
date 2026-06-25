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

// Pinos do XIAO ESP32-C5 (board com poucos GPIOs, ver datasheet Seeed)
#define DHT_PIN 25     // D2 - GPIO25, livre, sem funcao alternativa
// MH-Z19C: o chicote em campo só tem VCC, GND e o fio amarelo (PWM) ligados
// - os fios de UART (RX/TX) foram cortados para economizar espaço na case.
// Por isso a leitura é feita via PWM, não via UART. Ver Docs/CODEBASE_MEMORY.md.
#define MHZ_PWM_PIN 12     // D7 - fio amarelo do MH-Z19C
#define MHZ_RANGE_PPM 5000 // faixa de detecção configurada no sensor (0-5000ppm)
#define STATUS_LED 27      // LED amarelo onboard do XIAO ESP32-C5

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

void setup()
{
    Serial.begin(115200);

    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    pinMode(MHZ_PWM_PIN, INPUT);

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

// Le o CO2 do MH-Z19C via PWM (fio amarelo - unico sinal disponivel no
// chicote real, ja que os fios de UART foram cortados). Formula do
// datasheet Winsen: Cppm = Range * (Th - 2ms) / (Th + Tl - 4ms),
// ciclo de ~1004ms.
int readCO2()
{
    unsigned long thUs = pulseIn(MHZ_PWM_PIN, HIGH, 1100000UL);
    unsigned long tlUs = pulseIn(MHZ_PWM_PIN, LOW, 1100000UL);

    if (thUs == 0 || tlUs == 0)
    {
        Serial.println("Falha ao ler o CO2 via PWM (timeout).");
        return -1;
    }

    float thMs = thUs / 1000.0;
    float tlMs = tlUs / 1000.0;

    return (int)(MHZ_RANGE_PPM * (thMs - 2.0) / (thMs + tlMs - 4.0));
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
