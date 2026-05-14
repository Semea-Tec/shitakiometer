.PHONY: help up down restart logs clean mock

help:
	@echo "Comandos disponiveis no Shitakiometer:"
	@echo "  make up          - Sobe todos os servicos (Mosquitto, InfluxDB, Grafana, mqtt-bridge)"
	@echo "  make down        - Para todos os servicos"
	@echo "  make restart     - Reinicia todos os servicos"
	@echo "  make logs        - Mostra os logs dos servicos em tempo real"
	@echo "  make clean       - Remove todos os volumes e dados armazenados (CUIDADO: apaga historico)"
	@echo "  make mock        - Executa o script gerador de dados mockados (Python)"
	@echo "  make connect-esp - Conecta ao Wi-Fi do ESP32 e configura o IP estatico 192.168.4.2"

up:
	docker-compose up -d --build

down:
	docker-compose down

restart:
	docker-compose restart

logs:
	docker-compose logs -f

clean:
	docker-compose down -v

mock:
	python python-mock/mock_sensor.py

connect-esp:
	@echo "Conectando a rede do ESP32..."
	nmcli device wifi connect "ESP32_Shitakiometer" password "password123"
	@echo "Configurando IP estatico 192.168.4.2..."
	nmcli connection modify "ESP32_Shitakiometer" ipv4.addresses 192.168.4.2/24 ipv4.method manual
	nmcli connection up "ESP32_Shitakiometer"
	@echo "Servidor conectado com sucesso no IP 192.168.4.2!"
