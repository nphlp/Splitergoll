.DEFAULT_GOAL := help

.PHONY: help setup build

help:
	@printf '%s\n' 'make setup : préparer le build local' 'make build : compiler les deux moitiés'

setup:
	@sh scripts/setup.sh

build:
	@sh scripts/build.sh
