# home-server — convenience wrappers around docker compose & scripts.
# Run `make help` for the list.
.DEFAULT_GOAL := help
SHELL := /bin/bash

## up: Start the whole stack (detached)
up:
	docker compose up -d --remove-orphans

## down: Stop the whole stack
down:
	docker compose down

## pull: Pull the latest images for all services
pull:
	docker compose pull

## restart: Recreate all containers
restart: down up

## ps: Show container status
ps:
	docker compose ps

## logs: Tail logs for all services (or one: make logs s=pihole)
logs:
	docker compose logs -f --tail=100 $(s)

## config: Render and validate the fully-merged compose config
config:
	docker compose config

## install: Bootstrap a fresh Ubuntu host (Docker, packages, network)
install:
	./scripts/install.sh

## update: Pull new images and recreate changed containers
update:
	./scripts/update.sh

## backup: Snapshot configs + volumes to ./backups/
backup:
	./scripts/backup.sh

## restore: Restore from a backup archive (make restore f=backups/xyz.tar.gz)
restore:
	./scripts/restore.sh $(f)

## help: Show this help
help:
	@grep -E '^## ' $(MAKEFILE_LIST) | sed 's/## //' | awk -F': ' '{printf "  \033[36m%-10s\033[0m %s\n", $$1, $$2}'

.PHONY: up down pull restart ps logs config install update backup restore help
