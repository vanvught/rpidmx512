EXTRA_INCLUDES+=


ifneq ($(MAKE_FLAGS),)
else
	DEFINES+=OUTPUT_DMX_PIXEL_MULTI PIXELPATTERNS_MULTI
endif