ifneq (,$(filter wot_core,$(USEMODULE)))
  DIRS += core
  DIRS += core/serialization
endif
ifneq (,$(filter wot_coap,$(USEMODULE)))
  DIRS += coap
endif
ifneq (,$(filter wot_coap_config,$(USEMODULE)))
  DIRS += coap_config
endif

include $(RIOTBASE)/Makefile.base
