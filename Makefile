.DEFAULT_GOAL := help

.PHONY: help setup build flash bat usb-capture stop clear

# Récupère les mots après la commande et protège chaque argument pour le shell.
ARGS = $(foreach arg,$(filter-out $@,$(MAKECMDGOALS)),'$(subst ','"'"',$(arg))')

help:
	@bash scripts/help.sh $(ARGS)

setup:
	@bash scripts/setup.sh $(ARGS)

build:
	@bash scripts/build.sh $(ARGS)

flash:
	@bash scripts/flash.sh $(ARGS)

bat:
	@bash scripts/batteries.sh $(ARGS)

usb-capture:
	@bash diagnostics/os-signature/capture.sh $(ARGS)

stop:
	@sh scripts/stop.sh $(ARGS)

clear:
	@sh scripts/clear.sh $(ARGS)

# Make voit aussi wire, bluetooth et avg comme des cibles. Le script reçoit déjà ces mots.
%:
	@:
