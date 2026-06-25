# Memória do Projeto (Hardware & Firmware)

Fonte de verdade compartilhável sobre decisões de hardware/firmware do
shitakiometer. Complementa (não substitui) `Docs/documentacao.md` e os
READMEs específicos de cada pasta.

## Arquitetura atual (sensor dividido do transmissor LoRa)

```
[Sensores: DHT22 + MH-Z19C]
        |
        v
[XIAO ESP32-C5]  --ESP-NOW broadcast (2.4GHz)-->  [Heltec WiFi LoRa 32 V3]
   "sensor"                                          "concentrador"
                                                            |
                                                            v LoRa P2P (915MHz)
                                                  [Receptor LoRa / Gateway]
                                                  (acessível via celular: HTTP/MQTT)
```

- **Sensor:** [`Tests/Network/sensor/sensor.ino`](../Tests/Network/sensor/sensor.ino) — Seeed Studio XIAO ESP32-C5.
- **Concentrador:** [`Tests/Network/sender/concentradorHeltec.ino`](../Tests/Network/sender/concentradorHeltec.ino) — Heltec WiFi LoRa 32 V3 (ESP32-S3).
- **Receptor/Gateway:** [`Tests/Network/receiver/receiver_webServer.ino`](../Tests/Network/receiver/receiver_webServer.ino) ou [`receiver_mqtt.ino`](../Tests/Network/receiver/receiver_mqtt.ino) — sem alterações.

### Por que dividir sensor e LoRa em dois boards?

Anteriormente (`senderWithSensors.ino`) um único Heltec V3 lia os sensores
e transmitia via LoRa. Ao trocar o dispositivo de entrada pelo XIAO
ESP32-C5 — que tem muito menos GPIOs e **não** possui rádio LoRa nem OLED
onboard — não é possível replicar o SPI do SX1262 nesse board. A solução
foi separar as responsabilidades, o que já era a arquitetura descrita em
[`Tests/Network/README.md`](../Tests/Network/README.md) (camada de coleta
via ESP-NOW → camada de borda com LoRa):

- O **XIAO ESP32-C5** vira o nó sensor: só lê DHT22/MH-Z19C e envia via
  **ESP-NOW broadcast** (usa apenas o rádio 2.4GHz nativo do chip — não
  consome nenhum GPIO extra, ideal dado o número reduzido de pinos).
- O **Heltec V3** vira o concentrador: recebe via ESP-NOW e retransmite
  via LoRa, mantendo o mesmo formato de payload (`t:..,h:..,co2:..`), então
  os receivers existentes não precisaram de nenhuma alteração.
- Usamos **broadcast** (`FF:FF:FF:FF:FF:FF`) ao invés de unicast para não
  depender de descobrir/fixar o MAC address do Heltec — facilita a
  substituição de qualquer um dos dois boards no futuro.

`senderWithSensors.ino` foi mantido como referência/fallback (modo
standalone, só com o Heltec, sem o XIAO) — não é mais o caminho principal.

## Seeed Studio XIAO ESP32-C5 — pinagem usada

Board com poucos GPIOs expostos (D0-D10). Mapeamento usado em
`sensor.ino`:

| Função              | Pino do board | GPIO    | Observação                                  |
|---------------------|---------------|---------|----------------------------------------------|
| DHT22 (dado)         | D2            | GPIO25  | GPIO livre, sem função alternativa relevante |
| MH-Z19C RX (do XIAO)| D7            | GPIO12  | Recebe o TX do sensor de CO2                 |
| MH-Z19C TX (do XIAO)| D6            | GPIO11  | Envia para o RX do sensor de CO2             |
| LED de status        | onboard       | GPIO27  | LED amarelo onboard do XIAO                  |

Pinos **não usados** neste projeto, mas disponíveis no board (referência
para expansão futura): D0/GPIO1 (ADC), D1/GPIO0, D3/GPIO7, D4/GPIO23 (I2C
SDA), D5/GPIO24 (I2C SCL), D8/GPIO8, D9/GPIO9, D10/GPIO10 (SPI, caso um
dia seja necessário ligar outro periférico SPI). Os pinos JTAG
(MTDO/MTDI/MTCK/MTMS — GPIO5/3/4/2) foram evitados de propósito: a Seeed
recomenda não reaproveitá-los para evitar problemas de debug/boot.

### Ligações físicas (XIAO ESP32-C5)

- DHT22: `VCC` → 3V3, `GND` → GND, `DATA` → D2.
- MH-Z19C: alimentar com **5V** (não 3.3V — sensor não funciona
  corretamente em 3.3V), `GND` → GND, `TX` (do sensor) → D7, `RX` (do
  sensor) → D6.

### Setup do Arduino IDE para o XIAO ESP32-C5

1. Instalar o pacote de boards **"esp32" (by Espressif Systems)** versão
   **3.3.5 ou superior** via Boards Manager (não é o pacote da Heltec).
2. Selecionar a placa **"XIAO_ESP32C5"**.
3. Bibliotecas necessárias: `DHT22` (mesma usada no Heltec).
   `esp_now.h`/`esp_mac.h`/`WiFi.h` já vêm com o core do ESP32, não
   precisam de instalação extra.

## Heltec WiFi LoRa 32 V3 — concentrador

Sem mudanças de pinagem em relação ao que já estava documentado em
[`Lora/README.md`](../Lora/README.md): SPI do LoRa em GPIO 8-14, OLED I2C
em GPIO 17/18/21, Vext em GPIO 36, LED branco em GPIO 35. A única
mudança de firmware foi trocar a leitura direta dos sensores por um
callback `esp_now_register_recv_cb`, mantendo igual a inicialização do
LoRa/OLED e o formato de payload transmitido.

**Atenção:** o concentrador precisa estar no mesmo canal Wi-Fi (canal 0 =
canal atual do board) que o sensor para receber o broadcast ESP-NOW —
como nenhum dos dois se conecta a uma rede Wi-Fi, ambos usam o canal
padrão e isso não costuma ser um problema, mas se algum dos boards rodar
outro firmware que force conexão Wi-Fi STA a um AP, o canal pode mudar e
quebrar o ESP-NOW.

## Formato de payload (inalterado)

Mantido para compatibilidade com os receivers existentes:

```
t:<temperatura em °C, 1 decimal>,h:<umidade em %, 1 decimal>,co2:<ppm, inteiro>
```

Exemplo: `t:25.5,h:60.2,co2:412`. Quando uma leitura falha, o campo
correspondente é enviado como `-1` (CO2) ou `nan` (temp/hum, tratado nos
receivers/no OLED como "ERRO").
