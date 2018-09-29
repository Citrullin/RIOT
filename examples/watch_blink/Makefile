# name of your application
APPLICATION = iota

# Use the ST B-L072Z-LRWAN1 board by default:
BOARD ?= nrf52dk
#BOARD ?= yunjia-nrf51822
#BOARD ?= bluepill

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += xtimer

USEMODULE += shell
USEMODULE += shell_commands
# additional modules for debugging:
USEMODULE += ps

# Include
#USEPKG += iota-wallet
USEPKG += nordic_softdevice_ble

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
