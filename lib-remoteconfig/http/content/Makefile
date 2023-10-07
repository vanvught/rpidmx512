PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../../..
INCLUDES := -I$(ROOT)/lib-network/include -I$(ROOT)/lib-remoteconfig/include
COPS := -Wall -Werror

all : generate_content

clean : 
	rm generate_content
	
content : generate_content
	./generate_content
	
generate_content : Makefile generate_content.cpp
	$(CPP) generate_content.cpp $(INCLUDES) $(COPS) -o generate_content 