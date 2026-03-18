PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../../..
INCLUDES := -I$(ROOT)/common/include -I$(ROOT)/lib-remoteconfig/include -I$(ROOT)/lib-hal/include -I$(ROOT)/lib-network/include
COPS := -std=c++20 -Wall -Werror

all : content

clean : 
	rm -rf generate_content
	rm -rf *.h
		
generate_content : Makefile generate_content.cpp
	$(CPP) generate_content.cpp $(INCLUDES) $(COPS) -o generate_content 
	
content : generate_content
	./generate_content
