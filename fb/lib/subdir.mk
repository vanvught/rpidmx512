################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../console.c \
../fb.c \
../font.c \
../mbox.c 

OBJS += \
./console.o \
./fb.o \
./font.o \
./mbox.o 

C_DEPS += \
./console.d \
./fb.d \
./font.d \
./mbox.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -DBUILDING_RPIBOOT -DENABLE_DEFAULT_FONT -DENABLE_FRAMEBUFFER -I"/opt/data/development/workspace/fb" -I"/opt/data/development/workspace/fb/include" -O3 -Wall -c -fmessage-length=0 -std=c99 -nostdlib -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mhard-float -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


