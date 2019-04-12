# name of your application
APPLICATION = iota

# Use the ST B-L072Z-LRWAN1 board by default:
BOARD ?= bluepill

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Include
USEPKG += iota-library
USEPKG += nanopb

# POSIX Socket API
USEMODULE += gnrc_sock_udp
USEMODULE += posix_sockets

# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_udp

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
