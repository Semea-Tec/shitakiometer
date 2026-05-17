#include <esp_now.h>
#include <esp_mac.h>
#include <WiFi.h>
#include <DHT22.h>
#include <MHZ.h>

#define DHT_PIN 4

// Pinos para leitura do CO2

#define CO2_IN 5 // Pino PWM

DHT22 dhtSensor(DHT_PIN);

MHZ co2(CO2_IN, MHZ19C);

// SUBSTITUA PELO MAC QUE VOCÊ ANOTOU
uint8_t broadcastAddress[] = {0xF0, 0x9E, 0x9E, 0x78, 0x7B, 0x00};

typedef struct struct_message {
    uint8_t mac[6];
    float temp;
    int ppmCo2;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  pinMode(CO2_IN, INPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  // Registrar o dispositivo receptor (peer)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Falha ao adicionar peer");
    return;
  }

  esp_err_t ret = esp_read_mac(myData.mac, ESP_MAC_WIFI_STA);

  if (ret == ESP_OK) {
    Serial.printf("MAC capturado com sucesso: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  myData.mac[0], myData.mac[1], myData.mac[2], myData.mac[3], myData.mac[4], myData.mac[5]);
  } else {
    Serial.println("Erro crítico: Não foi possível ler o MAC da memória.");
  }

  // Sensor MHZ precisa esquentar antes de funcionar corretamente
  if (co2.isPreHeating()) {
    Serial.print("Preheating");
    while (co2.isPreHeating()) {
      Serial.print(".");
      delay(5000);
    }
    Serial.println();
  }
}

void loop() {
  float temp = dhtSensor.getTemperature();
  int ppmCo2 = co2.readCO2PWM();

  
  // Checa se a leitura é válida (nan = Not a Number)
  if (isnan(temp) || isnan(ppmCo2)) {
    Serial.println("Falha na leitura do sensor DHT22!");
  } else {
    myData.temp = temp;
    myData.ppmCo2 = ppmCo2;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.printf("MAC Remetente: %02X:%02X... | Enviado: %.2f°C\n", myData.mac[0], myData.temp);
    } else {
      Serial.println("Erro ao enviar via ESP-NOW");
    }
  }

  delay(2000);
}