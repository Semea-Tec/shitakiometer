import time
import random
import paho.mqtt.client as mqtt

# Configurações do broker MQTT
# Se rodar via docker-compose (como outro serviço), use "mosquitto"
# Se rodar direto no seu Windows, use "localhost"
MQTT_BROKER = "localhost" 
MQTT_PORT = 1883

def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print(f"Conectado ao broker MQTT em {MQTT_BROKER}:{MQTT_PORT}!")
    else:
        print(f"Falha na conexão. Código: {rc}")

# Trata a compatibilidade com paho-mqtt 1.x e 2.x
if hasattr(mqtt, 'CallbackAPIVersion'):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, "MockSensorClient")
else:
    client = mqtt.Client("MockSensorClient")

client.on_connect = on_connect

try:
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_start()

    print("Iniciando o envio de dados mockados (Pressione Ctrl+C para parar)...")
    
    # Valores iniciais base
    temperatura = 25.0
    umidade = 60.0

    while True:
        # Simula uma leve variação nos valores
        temperatura += random.uniform(-0.5, 0.5)
        umidade += random.uniform(-1.0, 1.0)
        
        # Limita para não ter valores absurdos
        temperatura = max(15.0, min(35.0, temperatura))
        umidade = max(30.0, min(90.0, umidade))

        # Publica no MQTT
        print(f"Publicando -> Temperatura: {temperatura:.2f} °C | Umidade: {umidade:.2f} %")
        client.publish("sensores/temperatura", f"{temperatura:.2f}")
        client.publish("sensores/umidade", f"{umidade:.2f}")

        # Aguarda 5 segundos antes de enviar o próximo dado
        time.sleep(5)

except KeyboardInterrupt:
    print("\nParando o envio de dados mockados.")
except Exception as e:
    print(f"Erro: {e}")
finally:
    client.loop_stop()
    client.disconnect()
