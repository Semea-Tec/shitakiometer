.PHONY: help up down restart logs clean mock

help:
	@echo "Comandos disponiveis no Shitakiometer:"
	@echo "  make up      - Sobe todos os servicos (Mosquitto, InfluxDB, Grafana, mqtt-bridge)"
	@echo "  make down    - Para todos os servicos"
	@echo "  make restart - Reinicia todos os servicos"
	@echo "  make logs    - Mostra os logs dos servicos em tempo real"
	@echo "  make clean   - Remove todos os volumes e dados armazenados (CUIDADO: apaga historico)"
	@echo "  make mock    - Executa o script gerador de dados mockados (Python)"

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