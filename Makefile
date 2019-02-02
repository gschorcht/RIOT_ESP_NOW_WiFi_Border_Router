# name of your application
APPLICATION = RIOT_ESP_NOW_WiFi_Border_Router

# If no BOARD is found in the environment, use this default:
BOARD ?= esp32-wroom-32

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# platform specific modules and settings
GNRC_NETIF_NUMOF := 2
USEMODULE += esp_wifi
USEMODULE += esp_now

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for 6LoWPAN border router
USEMODULE += gnrc_sixlowpan_border_router_default
# Add forwarding table
USEMODULE += fib
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Optionally include RPL as a routing protocol.
#USEMODULE += gnrc_rpl

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Prefix has to be /64
IPV6_PREFIX ?= 2001:db8::

# Outer address of the router and the length of its prefix if IPV6_AUTO=0
IPV6_AUTO ?= 1
IPV6_ADDR ?= fd19:aaaa::1
IPV6_ADDR_LEN ?= 64
IPV6_DEF_RT ?= fd19:aaaa::2

# Pass as CFLAGS to program
CFLAGS += -DBR_IPV6_PREFIX=\"$(IPV6_PREFIX)\"
ifeq (0,$(IPV6_AUTO))
  CFLAGS += -DBR_IPV6_ADDR=\"$(IPV6_ADDR)\" -DBR_IPV6_ADDR_LEN=$(IPV6_ADDR_LEN) -DBR_IPV6_DEF_RT=\"$(IPV6_DEF_RT)\"
endif

# Might need more than the default of 2 addresses.
CFLAGS += -DGNRC_NETIF_IPV6_ADDRS_NUMOF=4

include $(RIOTBASE)/Makefile.include
