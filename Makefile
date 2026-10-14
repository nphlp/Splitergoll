.DEFAULT_GOAL := help

.PHONY: help setup build wire bluetooth stop clear

# Make lit le profil comme une seconde cible. build transmet ce nom au script.
ifneq ($(filter build,$(MAKECMDGOALS)),)
ifneq ($(filter-out build wire bluetooth,$(MAKECMDGOALS)),)
$(error Utilise make build wire ou make build bluetooth)
endif
endif
ifneq ($(filter wire bluetooth,$(MAKECMDGOALS)),)
ifeq ($(filter build,$(MAKECMDGOALS)),)
$(error Utilise make build wire ou make build bluetooth)
endif
endif

help:
	@printf '%s\n' \
		'make setup : préparer le build local' \
		'make build wire : compiler le profil USB' \
		'make build bluetooth : compiler le profil Bluetooth direct' \
		'make build : compiler wire puis bluetooth' \
		'make stop : arrêter les conteneurs de build de ce checkout' \
		'make clear : arrêter le build et supprimer les sources, caches et firmwares locaux'

setup:
	@bash scripts/setup.sh

build:
	@bash scripts/build.sh $(filter wire bluetooth,$(MAKECMDGOALS))

wire:
	@:

bluetooth:
	@:

stop:
	@sh scripts/stop.sh

clear:
	@sh scripts/clear.sh
