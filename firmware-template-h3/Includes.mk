$(info "Includes.mk")
$(info $$PROJECT [${PROJECT}])


INCLUDES:=-I./include -I../include
INCLUDES+=-I../${PROJECT}/include
INCLUDES+=-I../common/include -I../common/firmware/include
INCLUDES+=-I../firmware-template-h3/include
INCLUDES+=-I../lib-h3/CMSIS/Core_A/Include
INCLUDES+=-I../lib-hal/include
 
INCLUDE_DIRS = $(shell find ../ -type d -name include | grep lib | grep -vE 'bcm|hal' | sort)
INCLUDES+=$(addprefix -I,$(INCLUDE_DIRS))

$(info $$INCLUDES [${INCLUDES}])
