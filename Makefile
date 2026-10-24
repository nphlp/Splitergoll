.DEFAULT_GOAL := help

##############
# Variables  #
##############

COMMANDS := help setup build flash stop clear bat app port usb ble
COMMAND := $(or $(firstword $(MAKECMDGOALS)),help)
ARGUMENTS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
ARGS = $(foreach arg,$(ARGUMENTS),'$(subst ','"'"',$(arg))')

# Seule la première commande lance Bash. Ainsi, build reste un argument dans make app build.
SCRIPT = $(if $(filter $@,$(COMMAND)),bash,:)
.PHONY: $(COMMANDS)

##############
#    Help    #
##############

help:
	@$(SCRIPT) scripts/help.sh $(ARGS)

################
#   Firmware   #
################

setup:
	@$(SCRIPT) scripts/setup.sh $(ARGS)

build:
	@$(SCRIPT) scripts/build.sh $(ARGS)

flash:
	@$(SCRIPT) scripts/flash.sh $(ARGS)

stop:
	@$(SCRIPT) scripts/stop.sh $(ARGS)

clear:
	@$(SCRIPT) scripts/clear.sh $(ARGS)

################
#  Batteries   #
################

bat:
	@$(SCRIPT) scripts/batteries.sh $(ARGS)

app:
	@$(SCRIPT) companion/app.sh $(ARGS)

##############
#   Debug    #
##############

port:
	@$(SCRIPT) scripts/help.sh port $(ARGS)

usb:
	@$(SCRIPT) diagnostics/os-signature/usb.sh $(ARGS)

ble:
	@$(SCRIPT) diagnostics/os-signature/ble.sh $(ARGS)

################
#  Arguments   #
################

# Les arguments sont des cibles sans action, y compris si le mot désigne un fichier existant.
EXTRA_ARGUMENTS := $(filter-out $(COMMANDS),$(ARGUMENTS))
ifneq ($(strip $(EXTRA_ARGUMENTS)),)
.PHONY: $(EXTRA_ARGUMENTS)
$(EXTRA_ARGUMENTS):
	@:
endif

# Une commande inconnue affiche l’aide générale.
%:
	@$(SCRIPT) scripts/help.sh
