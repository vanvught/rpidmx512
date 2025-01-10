
EXTRA_SRCDIR+=src/c++

ifneq ($(MAKE_FLAGS),)
	ifeq (,$(findstring CONFIG_HAVE_CRC32_HW,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/crc32
	endif
else
	EXTRA_SRCDIR+=src/crc32
endif