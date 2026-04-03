PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../../..
INCLUDES := -I$(ROOT)/common/include -I$(ROOT)/lib-remoteconfig/include -I$(ROOT)/lib-hal/include -I$(ROOT)/lib-network/include
COPS := -std=c++20 -Wall -Werror

CONTENT ?= light

all : content

clean : 
	rm -rf generate_content
	rm -rf *.h
	rm -rf ._*.*
	rm -rf ._*
		
generate_content : Makefile generate_content.cpp
	$(CPP) generate_content.cpp $(INCLUDES) $(COPS) -o generate_content
	
pixeltype : $(ROOT)/lib-pixel/include/pixeltype.h
	./parse_pixeltype.py $(ROOT)/lib-pixel/include/pixeltype.h -o pixeltype.json
	
content : generate_content pixeltype
	@echo "Running with directory $(CONTENT)"
	./generate_content $(CONTENT)
