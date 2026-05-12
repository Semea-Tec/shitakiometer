# Infraestrutura do Servidor (IoT Stack) - Shitakiometer

Este diretório contém a configuração Docker para subir uma stack completa de monitoramento IoT, integrando MQTT, um serviço de ponte (bridge) customizado em Python, banco de dados temporal (InfluxDB) e dashboards automáticos no Grafana.

## Arquitetura e Serviços

| Serviço | Porta Externa | Função |
|---------|---------------|--------|
| **Mosquitto** | 1883 / 9001 | Broker MQTT para receber dados dos sensores (`sensores/#`). |
| **mqtt-bridge** | Interna | Script Python que ouve os dados do MQTT e salva no InfluxDB automaticamente. |
| **InfluxDB** | 8086 | Banco de dados Time-Series para armazenar histórico. |
| **Grafana** | 3000 | Visualização de dados e Dashboards (Provisionamento automático). |

---

## Como Rodar

Para facilitar o dia a dia, preparamos um **Makefile** com os principais comandos.
Se você estiver no Windows, pode rodar os comandos do `docker-compose` manualmente caso não tenha o `make` instalado.

### Usando o Makefile (Recomendado se suportado)

1. **Subir a infraestrutura:**
   ```bash
   make up
   ```
2. **Visualizar os logs (acompanhar os dados chegando):**
   ```bash
   make logs
   ```
3. **Parar os serviços:**
   ```bash
   make down
   ```

*(Veja `make help` para todos os comandos, incluindo o `make clean` para resetar os bancos de dados).*

### Usando o Docker Compose diretamente
- Para subir: `docker-compose up -d --build`
- Para ver logs: `docker-compose logs -f`
- Para descer: `docker-compose down`

---

## Acesso aos Painéis

### 1. Grafana (Visualização)
O Grafana já foi configurado com **Provisionamento Automático**, ou seja, ele já se conecta ao banco e já traz um Dashboard pronto assim que é iniciado!

- **Acesso:** http://localhost:3000
- **Usuário:** `admin`
- **Senha:** `admin` *(solicitará troca no primeiro acesso)*

Vá em **Dashboards** no menu lateral, e você verá o painel **Sensores IoT** com gráficos para Temperatura e Humidade já funcionando.

### 2. InfluxDB (Banco de Dados)
Caso queira visualizar os dados brutos ou criar consultas (Flux Queries) avançadas:

- **Acesso:** http://localhost:8086
- **Usuário:** `admin`
- **Senha:** `adminpassword`
- **Organização:** `shitakiometer`
- **Bucket:** `sensores`

O Token da API para integrações externas já está fixado via variáveis de ambiente (`DOCKER_INFLUXDB_INIT_ADMIN_TOKEN`), tornando a comunicação interna muito mais fácil.

---

## Como Testar sem o Hardware (Dados Mockados)

Se você não tiver seu microcontrolador conectado, você pode gerar dados simulados (mock) para ver os gráficos do Grafana ganhando vida:

1. Certifique-se de que os contêineres Docker estão rodando.
2. Instale as dependências locais de Python (necessário apenas na sua máquina, caso queira rodar o mock localmente):
   ```bash
   pip install paho-mqtt
   ```
3. Execute o script simulador:
   ```bash
   python mock_sensor.py
   # Ou use: make mock
   ```

O script começará a publicar valores flutuantes de Temperatura e Humidade a cada 5 segundos no tópico `sensores/temperatura` e `sensores/umidade`. Aperte `Ctrl + C` para parar.