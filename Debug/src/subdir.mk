################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/hw_config.cpp \
../src/main.cpp \
../src/msf.cpp \
../src/msg.cpp \
../src/stm3210b_lctech.cpp \
../src/stm32_it.cpp \
../src/system_stm32f10x.cpp \
../src/systick.cpp \
../src/usb_desc.cpp \
../src/usb_endp.cpp \
../src/usb_istr.cpp \
../src/usb_prop.cpp \
../src/usb_pwr.cpp 

S_SRCS += \
../src/startup_stm32f10x_md.s 

OBJS += \
./src/hw_config.o \
./src/main.o \
./src/msf.o \
./src/msg.o \
./src/startup_stm32f10x_md.o \
./src/stm3210b_lctech.o \
./src/stm32_it.o \
./src/system_stm32f10x.o \
./src/systick.o \
./src/usb_desc.o \
./src/usb_endp.o \
./src/usb_istr.o \
./src/usb_prop.o \
./src/usb_pwr.o 

CPP_DEPS += \
./src/hw_config.d \
./src/main.d \
./src/msf.d \
./src/msg.d \
./src/stm3210b_lctech.d \
./src/stm32_it.d \
./src/system_stm32f10x.d \
./src/systick.d \
./src/usb_desc.d \
./src/usb_endp.d \
./src/usb_istr.d \
./src/usb_prop.d \
./src/usb_pwr.d 

S_DEPS += \
./src/startup_stm32f10x_md.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Assembler 5'
	armasm --cpu=Cortex-M3 -g --md --depend_format=unix_escaped -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


