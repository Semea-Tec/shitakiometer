# Infraestrutura do Servidor (IoT Stack)

Este diretório contém a configuração Docker para subir uma stack completa de monitoramento IoT, integrando MQTT, processamento de fluxo, banco de dados temporal e dashboards.

## Serviços

| Serviço | Porta Externa | Função |
|---------|---------------|--------|
| **Mosquitto** | 1883 / 9001 | Broker MQTT para receber dados dos sensores. |
| **Node-RED** | 1880 | Orquestração (Lê do MQTT e salva no InfluxDB). |
| **InfluxDB** | 8086 | Banco de dados Time-Series para armazenar histórico. |
| **Grafana** | 3000 | Visualização de dados e Dashboards. |

## Como Rodar

1. Certifique-se de ter o **Docker** e **Docker Compose** instalados.
2. Na pasta `server`, execute:
   ```bash
   docker-compose up -d
   ```
3. Verifique se os containers estão rodando:
   ```bash
   docker-compose ps
   ```

## Guia de Configuração e Acesso

### 1. InfluxDB (Banco de Dados)
O banco é inicializado automaticamente com as variáveis de ambiente definidas no `docker-compose.yml`.

- **Acesso:** http://localhost:8086
- **Usuário:** `admin`
- **Senha:** `adminpassword`
- **Organização:** `semea_tec`
- **Bucket:** `sensores`

> **Importante:** Ao acessar pela primeira vez, vá em **Data (Load Data) > API Tokens** e copie o **Admin's Token**. Você precisará dele para configurar o Node-RED e o Grafana. Caso não tenha acesso criei um novo token e salve-o.

### 2. Node-RED (Ponte MQTT -> InfluxDB)
Como o InfluxDB v2 não ingere MQTT nativamente, usamos o Node-RED para fazer essa ponte.

- **Acesso:** http://localhost:1880
- **Importação Rápida (Recomendado):**
  - Utilize o arquivo [`flows.json`](./flows.json) disponível neste diretório.
  - No Node-RED: **Menu > Import**, selecione o arquivo e confirme.
  - **Atenção:** Dê um duplo clique no nó do InfluxDB, edite a configuração do servidor (ícone de lápis) e cole o **Token** que você copiou no passo anterior.

- **Detalhes da Configuração (Manual):**
  - **Conexão com MQTT:**
    - Use um nó `mqtt in`.
    - Servidor: `mosquitto` (nome do container na rede docker).
    - Porta: `1883`.

  - **Transformação de Dados (Function):**
    - Adicione um nó `function` entre o MQTT e o InfluxDB.
    - É necessário **ligar** os nós: saída do MQTT -> entrada da Function -> entrada do InfluxDB.
    - Este nó transforma o dado bruto recebido em um objeto JSON com campos como `measurement` e `value` para o banco.
  
  - **Conexão com InfluxDB:**
    - Use um nó `influxdb out` (instale via *Manage Palette* se necessário: `node-red-contrib-influxdb`).
    - Versão: 2.0.
    - URL: `http://influxdb:8086`.
    - Token: (Token copiado do passo anterior).
    - Org: `semea_tec` / Bucket: `sensores`.

### 3. Grafana (Dashboards)
- **Acesso:** http://localhost:3000
- **Login inicial:** `admin` / `admin` (solicitará troca de senha).

**Adicionando Fonte de Dados (Data Source):**
1. Vá em **Connections > Data Sources > Add data source**.
2. Selecione **InfluxDB**.
3. Em **Query Language**, escolha **Flux**.
4. Em **HTTP > URL**, digite: `http://influxdb:8086` (comunicação interna docker).
5. Autenticação:
   - Organization: `semea_tec`
   - Token: (Token do InfluxDB)
   - Default Bucket: `sensores`
6. Clique em **Save & Test**.

### 4. Criando um Dashboard Simples
O método mais prático para criar gráficos é gerar a consulta no InfluxDB e importá-la no Grafana.

**Passo 1: Gerar a Query no InfluxDB**
1. Acesse o InfluxDB (http://localhost:8086) e vá em **Data Explorer** (ícone de gráfico).
2. Na parte inferior, selecione o bucket `sensores`.
3. Em `_measurement`, escolha `temperatura`.
4. Em `_field`, escolha `value`.
5. Clique em **Submit** para visualizar os dados brutos.
6. Clique no botão **Script Editor** e copie o código gerado (Linguagem Flux).

**Passo 2: Configurar no Grafana**
1. No Grafana, vá em **Dashboards > New** e clique em **Add visualization**.
2. Selecione a fonte de dados `InfluxDB` que você configurou.
3. Na área de código da query, cole o script que você copiou do InfluxDB.
4. Clique em **Run queries** (ou clique fora da caixa de texto).
5. Personalize o título e cores no menu lateral direito e clique em **Save**.

Agora você tem um dashboard exibindo os dados em tempo real vindos do MQTT.