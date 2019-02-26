# name of your application
APPLICATION = troika_benchmark

# Use the ST B-L072Z-LRWAN1 board by default:
BOARD ?= bluepill

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

CFLAGS += -Wno-unused-function -Wno-unused-but-set-variable -Wno-sign-compare

# Include
USEMODULE += xtimer
USEPKG += iota-library
USEPKG += troika-library

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include