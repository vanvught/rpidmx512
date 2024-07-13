PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../../..
INCLUDES := -I$(ROOT)/lib-hal/include -I$(ROOT)/lib-network/include -I$(ROOT)/lib-remoteconfig/include -I$(ROOT)/lib-configstore/include -I$(ROOT)/lib-debug/include
COPS := -std=c++11 -Wall -Werror

all : content generate_json_switch

clean : 
	rm -rf generate_content
	rm -rf generate_json_switch
	rm -rf *.h
		
generate_content : Makefile generate_content.cpp
	$(CPP) generate_content.cpp $(INCLUDES) $(COPS) -o generate_content 
	
content : generate_content generate_json_switch
	./generate_content
	./generate_json_switch
	
generate_json_switch : Makefile generate_json_switch.cpp
	$(CPP) generate_json_switch.cpp $(INCLUDES) $(COPS) -o generate_json_switch 