################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RVMSIS/core_riscv.c 

C_DEPS += \
./RVMSIS/core_riscv.d 

OBJS += \
./RVMSIS/core_riscv.o 

DIR_OBJS += \
./RVMSIS/*.o \

DIR_DEPS += \
./RVMSIS/*.d \

DIR_EXPANDS += \
./RVMSIS/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
RVMSIS/%.o: ../RVMSIS/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DCH583 -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/Startup" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/Profile/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/StdPeriphDriver/inc" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/HAL/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/Ld" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/LIB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/RVMSIS" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/app_drv_fifo" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/ble_uart_service" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/APP/PCB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/Ld" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/RVMSIS" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/Startup" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/SRC/StdPeriphDriver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

