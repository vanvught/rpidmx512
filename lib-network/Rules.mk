$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring ESP8266,$(MAKE_FLAGS)), ESP8266)
		COND=1
	endif
	ifndef COND
		ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)), NO_EMAC)
			EXTRA_SRCDIR+=src/noemac
			COND=1
		endif
	endif
	ifndef COND
		EXTRA_SRCDIR+=src/net src/apps src/emac src/params 
	endif
else
	EXTRA_SRCDIR+=src/net src/apps src/params src/emac
endif
