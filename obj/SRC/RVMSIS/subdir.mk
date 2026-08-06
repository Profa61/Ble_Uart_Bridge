################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SRC/RVMSIS/core_riscv.c 

C_DEPS += \
./SRC/RVMSIS/core_riscv.d 

OBJS += \
./SRC/RVMSIS/core_riscv.o 

DIR_OBJS += \
./SRC/RVMSIS/*.o \

DIR_DEPS += \
./SRC/RVMSIS/*.d \

DIR_EXPANDS += \
./SRC/RVMSIS/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
SRC/RVMSIS/%.o: ../SRC/RVMSIS/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DCH583 -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/Profile/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/HAL/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/LIB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/app_drv_fifo" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/ble_uart_service" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/PCB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/Ld" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/RVMSIS" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/Startup" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/StdPeriphDriver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

