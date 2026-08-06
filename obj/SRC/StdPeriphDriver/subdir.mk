################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SRC/StdPeriphDriver/CH58x_adc.c \
../SRC/StdPeriphDriver/CH58x_clk.c \
../SRC/StdPeriphDriver/CH58x_flash.c \
../SRC/StdPeriphDriver/CH58x_gpio.c \
../SRC/StdPeriphDriver/CH58x_i2c.c \
../SRC/StdPeriphDriver/CH58x_pwm.c \
../SRC/StdPeriphDriver/CH58x_pwr.c \
../SRC/StdPeriphDriver/CH58x_spi0.c \
../SRC/StdPeriphDriver/CH58x_spi1.c \
../SRC/StdPeriphDriver/CH58x_sys.c \
../SRC/StdPeriphDriver/CH58x_timer0.c \
../SRC/StdPeriphDriver/CH58x_timer1.c \
../SRC/StdPeriphDriver/CH58x_timer2.c \
../SRC/StdPeriphDriver/CH58x_timer3.c \
../SRC/StdPeriphDriver/CH58x_uart0.c \
../SRC/StdPeriphDriver/CH58x_uart1.c \
../SRC/StdPeriphDriver/CH58x_uart2.c \
../SRC/StdPeriphDriver/CH58x_uart3.c \
../SRC/StdPeriphDriver/CH58x_usb2dev.c \
../SRC/StdPeriphDriver/CH58x_usb2hostBase.c \
../SRC/StdPeriphDriver/CH58x_usb2hostClass.c \
../SRC/StdPeriphDriver/CH58x_usbdev.c \
../SRC/StdPeriphDriver/CH58x_usbhostBase.c \
../SRC/StdPeriphDriver/CH58x_usbhostClass.c 

C_DEPS += \
./SRC/StdPeriphDriver/CH58x_adc.d \
./SRC/StdPeriphDriver/CH58x_clk.d \
./SRC/StdPeriphDriver/CH58x_flash.d \
./SRC/StdPeriphDriver/CH58x_gpio.d \
./SRC/StdPeriphDriver/CH58x_i2c.d \
./SRC/StdPeriphDriver/CH58x_pwm.d \
./SRC/StdPeriphDriver/CH58x_pwr.d \
./SRC/StdPeriphDriver/CH58x_spi0.d \
./SRC/StdPeriphDriver/CH58x_spi1.d \
./SRC/StdPeriphDriver/CH58x_sys.d \
./SRC/StdPeriphDriver/CH58x_timer0.d \
./SRC/StdPeriphDriver/CH58x_timer1.d \
./SRC/StdPeriphDriver/CH58x_timer2.d \
./SRC/StdPeriphDriver/CH58x_timer3.d \
./SRC/StdPeriphDriver/CH58x_uart0.d \
./SRC/StdPeriphDriver/CH58x_uart1.d \
./SRC/StdPeriphDriver/CH58x_uart2.d \
./SRC/StdPeriphDriver/CH58x_uart3.d \
./SRC/StdPeriphDriver/CH58x_usb2dev.d \
./SRC/StdPeriphDriver/CH58x_usb2hostBase.d \
./SRC/StdPeriphDriver/CH58x_usb2hostClass.d \
./SRC/StdPeriphDriver/CH58x_usbdev.d \
./SRC/StdPeriphDriver/CH58x_usbhostBase.d \
./SRC/StdPeriphDriver/CH58x_usbhostClass.d 

OBJS += \
./SRC/StdPeriphDriver/CH58x_adc.o \
./SRC/StdPeriphDriver/CH58x_clk.o \
./SRC/StdPeriphDriver/CH58x_flash.o \
./SRC/StdPeriphDriver/CH58x_gpio.o \
./SRC/StdPeriphDriver/CH58x_i2c.o \
./SRC/StdPeriphDriver/CH58x_pwm.o \
./SRC/StdPeriphDriver/CH58x_pwr.o \
./SRC/StdPeriphDriver/CH58x_spi0.o \
./SRC/StdPeriphDriver/CH58x_spi1.o \
./SRC/StdPeriphDriver/CH58x_sys.o \
./SRC/StdPeriphDriver/CH58x_timer0.o \
./SRC/StdPeriphDriver/CH58x_timer1.o \
./SRC/StdPeriphDriver/CH58x_timer2.o \
./SRC/StdPeriphDriver/CH58x_timer3.o \
./SRC/StdPeriphDriver/CH58x_uart0.o \
./SRC/StdPeriphDriver/CH58x_uart1.o \
./SRC/StdPeriphDriver/CH58x_uart2.o \
./SRC/StdPeriphDriver/CH58x_uart3.o \
./SRC/StdPeriphDriver/CH58x_usb2dev.o \
./SRC/StdPeriphDriver/CH58x_usb2hostBase.o \
./SRC/StdPeriphDriver/CH58x_usb2hostClass.o \
./SRC/StdPeriphDriver/CH58x_usbdev.o \
./SRC/StdPeriphDriver/CH58x_usbhostBase.o \
./SRC/StdPeriphDriver/CH58x_usbhostClass.o 

DIR_OBJS += \
./SRC/StdPeriphDriver/*.o \

DIR_DEPS += \
./SRC/StdPeriphDriver/*.d \

DIR_EXPANDS += \
./SRC/StdPeriphDriver/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
SRC/StdPeriphDriver/%.o: ../SRC/StdPeriphDriver/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DCH583 -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/Profile/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/HAL/include" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/LIB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/app_drv_fifo" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/ble_uart_service" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/APP/PCB" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/Ld" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/RVMSIS" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/Startup" -I"c:/Users/v.tsaregorodtsev/Documents/vch/VCH_BLE/BLE_UART_BRIDGE/SRC/StdPeriphDriver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

