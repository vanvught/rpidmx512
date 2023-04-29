ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring ESP8266,$(MAKE_FLAGS)), ESP8266)
		EXTRA_SRCDIR+=src/esp8266	
	endif
else
	DEFINES+=ESP8266
	EXTRA_SRCDIR+=src/esp8266	
endif