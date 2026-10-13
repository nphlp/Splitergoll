.DEFAULT_GOAL := help

.PHONY: help setup build stop clear

help:
	@printf '%s\n' \
		'make setup : préparer le build local' \
		'make build : compiler les deux moitiés' \
		'make stop : arrêter les conteneurs de build de ce checkout' \
		'make clear : arrêter le build et supprimer les sources, caches et firmwares locaux'

setup:
	@sh scripts/setup.sh

build:
	@sh scripts/build.sh

stop:
	@sh scripts/stop.sh

clear:
	@sh scripts/clear.sh
