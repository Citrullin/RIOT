# name of your application
APPLICATION = saul_example

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

BOARD_INSUFFICIENT_MEMORY := arduino-duemilanove arduino-nano arduino-uno

# we want to use SAUL:
USEMODULE += saul
USEMODULE += saul_default
USEMODULE += saul_reg
USEMODULE += auto_init_saul
# include the shell:
USEMODULE += shell
USEMODULE += shell_commands
# additional modules for debugging:
USEMODULE += ps
USEMODULE += bme280
USEMODULE += tsl2561
USEMODULE += veml6070
USEMODULE += l3g4200d

CFLAGS += -DBMX280_PARAM_I2C_DEV=I2C_DEV\(0\)
CFLAGS += -DBMX280_PARAM_I2C_ADDR=0x76

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
