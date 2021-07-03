ifneq ($(MAKE_FLAGS),)
	ifeq (,$(findstring DISABLE_TFTP,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/h3/tftp
	endif
else
	ifeq (,$(findstring DISABLE_TFTP,$(DEFINES)))
		EXTRA_SRCDIR+=src/h3/tftp
	endif
	DEFINES+=LTC_READER
endif