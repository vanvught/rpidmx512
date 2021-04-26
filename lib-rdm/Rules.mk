ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_RDMNET_LLRP_ONLY,$(MAKE_FLAGS)))
		EXTRA_SRCDIR += src/llrp
	endif
else
	EXTRA_SRCDIR += src/llrp
endif