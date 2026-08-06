################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include StdPeriphDriver/subdir.mk
-include Startup/subdir.mk
-include RVMSIS/subdir.mk
-include Profile/subdir.mk
-include HAL/subdir.mk
-include APP/ble_uart_service/subdir.mk
-include APP/app_drv_fifo/subdir.mk
-include APP/PCB/subdir.mk
-include APP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
BLE_UART.hex \

SECONDARY_LIST += \
BLE_UART.lst \

SECONDARY_SIZE += \
BLE_UART.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: BLE_UART.elf secondary-outputs

# Tool invocations
BLE_UART.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -T "c:\Users\v.tsaregorodtsev\Documents\vch\VCH_BLE\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"c:\Users\v.tsaregorodtsev\Documents\vch\VCH_BLE" -L"c:\Users\v.tsaregorodtsev\Documents\vch\VCH_BLE\LIB" -L"c:\Users\v.tsaregorodtsev\Documents\vch\VCH_BLE\StdPeriphDriver" -Xlinker --print-memory-usage -Wl,-Map,"BLE_UART.map" --specs=nano.specs --specs=nosys.specs -o "BLE_UART.elf" $(OBJS) $(LIBS)
BLE_UART.hex: BLE_UART.elf
	@	riscv-none-embed-objcopy -O ihex "BLE_UART.elf" "BLE_UART.hex"
BLE_UART.lst: BLE_UART.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "BLE_UART.elf" > "BLE_UART.lst"
BLE_UART.siz: BLE_UART.elf
	riscv-none-embed-size --format=berkeley "BLE_UART.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) BLE_UART.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets