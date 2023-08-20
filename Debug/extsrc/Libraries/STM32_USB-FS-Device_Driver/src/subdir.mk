################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_core.c \
/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_init.c \
/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_int.c \
/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_mem.c \
/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_regs.c \
/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_sil.c 

OBJS += \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_core.o \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_init.o \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_int.o \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_mem.o \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_regs.o \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_sil.o 

C_DEPS += \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_core.d \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_init.d \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_int.d \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_mem.d \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_regs.d \
./extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_sil.d 


# Each subdirectory must supply rules for building sources it contributes
extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_core.o: /home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_core.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_init.o: /home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_init.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_int.o: /home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_int.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_mem.o: /home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_mem.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_regs.o: /home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_regs.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

extsrc/Libraries/STM32_USB-FS-Device_Driver/src/usb_sil.o: /home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib_V4.0.0/Libraries/STM32_USB-FS-Device_Driver/src/usb_sil.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM C++ Compiler 5'
	armcc --cpu=Cortex-M3 -DSTM32F10X_MD -DminiSTM32F103C8 -DUSE_STM3210B_LCTECH -DUSE_STDPERIPH_DRIVER -I"/home/tony/DS-5-Workspace/MSFTimer/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32_USB-FS-Device_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Device/ST/STM32F10x/Include" -I"/home/tony/DS-5-Workspace/STM32_USB-FS-Device_Lib/Libraries/CMSIS/Include" -O0 -g --md --depend_format=unix_escaped -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


