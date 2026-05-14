import os
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# Configurações do InfluxDB
INFLUXDB_URL = "http://influxdb:8086"
INFLUXDB_TOKEN = os.environ.get("INFLUXDB_TOKEN", "shitaki-super-secret-token")
INFLUXDB_ORG = "shitakiometer"
INFLUXDB_BUCKET = "sensores"

# Configurações do MQTT
MQTT_BROKER = "mosquitto"
MQTT_PORT = 1883
MQTT_TOPIC = "sensores/#"

# Inicializa o cliente do InfluxDB
influx_client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_TOKEN, org=INFLUXDB_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao broker MQTT com sucesso.")
        client.subscribe(MQTT_TOPIC)
        print(f"Inscrito no tópico: {MQTT_TOPIC}")
    else:
        print(f"Falha ao conectar ao broker MQTT. Código de retorno: {rc}")

def on_message(client, userdata, msg):
    try:
        # Extrai o nome da medição a partir do tópico
        # Exemplo: 'sensores/temperatura' -> 'temperatura'
        topic_parts = msg.topic.split('/')
        measurement = topic_parts[-1]
        
        # Lê o valor enviado na mensagem
        payload_str = msg.payload.decode("utf-8")
        value = float(payload_str)
        
        print(f"Recebido: {msg.topic} -> {value}")
        
        # Prepara os dados para o InfluxDB
        point = Point(measurement).field("value", value)
        
        # Salva no InfluxDB
        write_api.write(bucket=INFLUXDB_BUCKET, org=INFLUXDB_ORG, record=point)
        print(f"Salvo no InfluxDB: Measurement='{measurement}', Value={value}")

    except ValueError:
        print(f"Erro: O payload recebido no tópico '{msg.topic}' não é um número válido: {msg.payload}")
    except Exception as e:
        print(f"Erro ao processar mensagem: {e}")

# Configura o cliente MQTT
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

try:
    print(f"Tentando conectar ao broker MQTT em {MQTT_BROKER}:{MQTT_PORT}...")
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_forever()
except Exception as e:
    print(f"Erro fatal: {e}")
