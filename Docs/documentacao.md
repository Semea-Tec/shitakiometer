# Especificação de Status e Pendências: Sistema de Monitoramento de Estufa (Cogumelos)

**Projeto:** Monitoramento Ambiental de Alta Autonomia via LoRa
**Status:** Em Desenvolvimento (Fase de Integração de Hardware e Infraestrutura)

---

## 1. Visão Geral da Proposta

Este projeto visa a implementação de uma solução robusta para o monitoramento de variáveis críticas (Temperatura, Umidade e CO2) em estufas de produção de cogumelos. A solução foca em dois pilares principais: **longo alcance**, utilizando a tecnologia de comunicação LoRa, e **sustentabilidade energética**, através de alimentação solar, permitindo a instalação em locais remotos e sem infraestrutura de rede elétrica.

## 2. Objetivos Principais

* **Monitoramento Multiparamétrico:** Coleta precisa de temperatura, umidade relativa e níveis de dióxido de carbono.
* **Conectividade de Longo Alcance:** Implementação de protocolo LoRa para garantir a transmissão em áreas rurais extensas.
* **Autonomia Energética:** Sistema operado por painéis fotovoltaicos e baterias, visando zero manutenção de carga.

## 3. Especificação de Materiais

O sistema utiliza componentes de alta performance para garantir estabilidade:

* **Processamento e Rádio:** Heltec ESP32 com rádio LoRa integrado.
* **Sensores:** DHT22 (Clima), MH-Z19B (CO2) e SCD40 (CO2 de alta precisão).
* **Gestão Energética:** Placa solar 5V 1.25W, Bateria 18650 e Fonte 5V de suporte.

## 4. Progresso Atual (Milestones Concluídos)

Até a presente data, as seguintes frentes de trabalho foram finalizadas com sucesso:

### 4.1 Integração de Sensores

* A leitura de temperatura, umidade e CO2 foi validada via firmware.

### 4.2 Comunicação e Protocolos

* Estabelecimento de conexão ponto-a-ponto simplificada entre dispositivos LoRa.
* Validação de pacotes de dados básicos enviados via radiofrequência.

### 4.3 Infraestrutura de Backend (Stack de Dados)

A arquitetura de processamento de dados está operacional com a seguinte stack:

* **Mosquitto:** Broker MQTT configurado para orquestração de mensagens.
* **Node-RED:** Fluxos de automação estabelecidos para ponte entre MQTT e persistência.
* **InfluxDB:** Banco de dados de séries temporais otimizado para os logs dos sensores.
* **Grafana:** Dashboards preliminares criados para visualização em tempo real.

## 5. Pendências e Próximos Passos (Roadmap)

Para a conclusão do projeto e entrega da solução final:

### 5.1 Sistema de Alimentação Solar

* **Desenvolvimento:** Projetar e montar o circuito de carga e gestão de energia.
* **Eficiência:** Implementar estratégias de *Deep Sleep* no ESP32 para garantir que o consumo médio seja inferior à geração do painel de 1.25W.

### 5.2 Customização do Usuário Final (UX/Infraestrutura)

* **Adequação:** Ajustar as interfaces do Grafana e alertas do Node-RED para a linguagem e necessidades específicas do agricultor (ex: alertas de níveis críticos de CO2 via Telegram/WhatsApp).
* **Resiliência:** Configurar backups automáticos do banco de dados InfluxDB.
* Obs: Possibilidade da troca de tecnologias

### 5.3 Validação em Campo

* **Estabilidade:** Realizar testes de alcance e perda de pacotes diretamente no sítio (ambiente de produção real).
* **Confiabilidade:** Avaliar a vedação das caixas contra possíveis intempéries.

---

**Responsável pelo Documento:** Equipe de Desenvolvimento de Projetos IoT
