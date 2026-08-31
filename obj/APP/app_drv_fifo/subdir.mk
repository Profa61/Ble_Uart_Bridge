################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/app_drv_fifo/app_drv_fifo.c 

C_DEPS += \
./APP/app_drv_fifo/app_drv_fifo.d 

OBJS += \
./APP/app_drv_fifo/app_drv_fifo.o 

DIR_OBJS += \
./APP/app_drv_fifo/*.o \

DIR_DEPS += \
./APP/app_drv_fifo/*.d \

DIR_EXPANDS += \
./APP/app_drv_fifo/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
APP/app_drv_fifo/%.o: ../APP/app_drv_fifo/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -D__DEBUG=1 -DCH583 -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/Startup" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/Profile/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/StdPeriphDriver/inc" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/HAL/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/Ld" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/LIB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/RVMSIS" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/app_drv_fifo" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/ble_uart_service" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/PCB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/Ld" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/RVMSIS" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/Startup" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/StdPeriphDriver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

