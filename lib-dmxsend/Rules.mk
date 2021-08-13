ifneq ($(MAKE_FLAGS),)
	ifeq (,$(findstring OUTPUT_DMX_SEND_MULTI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/single
	else
		EXTRA_SRCDIR+=src/multi
	endif
else
	DEFINES+=OUTPUT_DMX_SEND_MULTI
	EXTRA_SRCDIR+=src/multi
endif