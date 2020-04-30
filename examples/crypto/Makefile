# name of your application
APPLICATION = saul_example

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# include the shell:
USEMODULE += shell
USEMODULE += shell_commands
# additional modules for debugging:
USEMODULE += ps
USEPKG += cryptoauthlib
USEMODULE += hashes

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
