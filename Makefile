# Project (adjust to match your needs)
PROJECT=blink
ELF = $(PROJECT).elf

# Library paths (adjust to match your needs)
STM32F4CUBE=$(ERS_ROOT)/stm32f4cube
CMSIS=$(STM32F4CUBE)/Drivers/CMSIS
HAL=$(STM32F4CUBE)/Drivers/STM32F4xx_HAL_Driver
HAL_BIN=bin

# Tools gcc + binutils + gdb + openocd
TOOLCHAIN_PREFIX:=arm-none-eabi-
CC=$(TOOLCHAIN_PREFIX)gcc
CXX=$(TOOLCHAIN_PREFIX)g++
LD=$(TOOLCHAIN_PREFIX)ld
OBJCOPY=$(TOOLCHAIN_PREFIX)objcopy
SIZE=$(TOOLCHAIN_PREFIX)size
GDB=/$(TOOLCHAIN_PREFIX)gdb
AS=$(TOOLCHAIN_PREFIX)as
OPENOCD=openocd

# Compiler and linker options
CFLAGS = -mcpu=cortex-m4 -g -Os -Wall -pipe
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MMD -MP -fsingle-precision-constant
CFLAGS += -D STM32F407xx
CXXFLAGS = $(CFLAGS) -std=c++14
LDFLAGS=-T STM32F407VG_FLASH.ld -specs=nosys.specs -Wl,-Map,$(PROJECT).map

# Includes including library includes
INCLUDES=\
-I./src \
-I./config \
-I$(HAL)/Inc \
-I$(CMSIS)/Device/ST/STM32F4xx/Include \
-I$(CMSIS)/Include

# Application objects
APP_OBJECTS=\
hardware/stm32f4xx_it.o \
hardware/system_stm32f4xx.o \
hardware/startup_stm32f407xx.o \
hardware/syscalls.o \
src/main.o \
src/LED.o \
src/Button.o

# Currenly used HAL module objects
HAL_OBJECTS=\
$(HAL_BIN)/stm32f4xx_hal.o \
$(HAL_BIN)/stm32f4xx_hal_gpio.o \
$(HAL_BIN)/stm32f4xx_hal_tim.o \
$(HAL_BIN)/stm32f4xx_hal_tim_ex.o \
$(HAL_BIN)/stm32f4xx_hal_rcc.o \
$(HAL_BIN)/stm32f4xx_hal_rcc_ex.o \
$(HAL_BIN)/stm32f4xx_hal_dma.o \
$(HAL_BIN)/stm32f4xx_hal_dma_ex.o \
$(HAL_BIN)/stm32f4xx_hal_cortex.o \
\
$(HAL_BIN)/stm32f4xx_hal_usart.o \
$(HAL_BIN)/stm32f4xx_hal_uart.o \

# Available HAL module objects
HAL_OBJECTS_EXTRA=\
$(HAL_BIN)/stm32f4xx_hal_wwdg.o \
$(HAL_BIN)/stm32f4xx_ll_fmc.o \
$(HAL_BIN)/stm32f4xx_ll_fsmc.o \
$(HAL_BIN)/stm32f4xx_ll_sdmmc.o \
$(HAL_BIN)/stm32f4xx_ll_usb.o \
$(HAL_BIN)/stm32f4xx_hal_hash.o \
$(HAL_BIN)/stm32f4xx_hal_hash_ex.o \
$(HAL_BIN)/stm32f4xx_hal_hcd.o \
$(HAL_BIN)/stm32f4xx_hal_i2c.o \
$(HAL_BIN)/stm32f4xx_hal_i2c_ex.o \
$(HAL_BIN)/stm32f4xx_hal_i2s.o \
$(HAL_BIN)/stm32f4xx_hal_i2s_ex.o \
$(HAL_BIN)/stm32f4xx_hal_irda.o \
$(HAL_BIN)/stm32f4xx_hal_iwdg.o \
$(HAL_BIN)/stm32f4xx_hal_lptim.o \
$(HAL_BIN)/stm32f4xx_hal_ltdc.o \
$(HAL_BIN)/stm32f4xx_hal_ltdc_ex.o \
$(HAL_BIN)/stm32f4xx_hal_nand.o \
$(HAL_BIN)/stm32f4xx_hal_nor.o \
$(HAL_BIN)/stm32f4xx_hal_pccard.o \
$(HAL_BIN)/stm32f4xx_hal_pcd.o \
$(HAL_BIN)/stm32f4xx_hal_pcd_ex.o \
$(HAL_BIN)/stm32f4xx_hal_pwr.o \
$(HAL_BIN)/stm32f4xx_hal_pwr_ex.o \
$(HAL_BIN)/stm32f4xx_hal_qspi.o \
$(HAL_BIN)/stm32f4xx_hal_rng.o \
$(HAL_BIN)/stm32f4xx_hal_rtc.o \
$(HAL_BIN)/stm32f4xx_hal_rtc_ex.o \
$(HAL_BIN)/stm32f4xx_hal_sai.o \
$(HAL_BIN)/stm32f4xx_hal_sai_ex.o \
$(HAL_BIN)/stm32f4xx_hal_sdram.o \
$(HAL_BIN)/stm32f4xx_hal_smartcard.o \
$(HAL_BIN)/stm32f4xx_hal_spdifrx.o \
$(HAL_BIN)/stm32f4xx_hal_spi.o \
$(HAL_BIN)/stm32f4xx_hal_sram.o \
$(HAL_BIN)/stm32f4xx_hal_adc.o \
$(HAL_BIN)/stm32f4xx_hal_adc_ex.o \
$(HAL_BIN)/stm32f4xx_hal_can.o \
$(HAL_BIN)/stm32f4xx_hal_cec.o \
$(HAL_BIN)/stm32f4xx_hal_crc.o \
$(HAL_BIN)/stm32f4xx_hal_cryp.o \
$(HAL_BIN)/stm32f4xx_hal_cryp_ex.o \
$(HAL_BIN)/stm32f4xx_hal_dac.o \
$(HAL_BIN)/stm32f4xx_hal_dac_ex.o \
$(HAL_BIN)/stm32f4xx_hal_dcmi.o \
$(HAL_BIN)/stm32f4xx_hal_dcmi_ex.o \
$(HAL_BIN)/stm32f4xx_hal_dfsdm.o \
$(HAL_BIN)/stm32f4xx_hal_dma2d.o \
$(HAL_BIN)/stm32f4xx_hal_dsi.o \
$(HAL_BIN)/stm32f4xx_hal_eth.o \
$(HAL_BIN)/stm32f4xx_hal_flash.o \
$(HAL_BIN)/stm32f4xx_hal_flash_ex.o \
$(HAL_BIN)/stm32f4xx_hal_flash_ramfunc.o \
$(HAL_BIN)/stm32f4xx_hal_fmpi2c.o \
$(HAL_BIN)/stm32f4xx_hal_fmpi2c_ex.o


OBJECTS=$(APP_OBJECTS) $(HAL_OBJECTS)

DEPENDENCIES=$(OBJECTS:.o=.d)

all: $(ELF)

clean:
	rm -f $(ELF)
	rm -f **/*.o
	rm -f **/*.d
	rm -f *.map
	rm -f $(HAL_OBJECTS)

# Link final efl
$(ELF): $(APP_OBJECTS) $(HAL_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Compile C source into object
$(HAL_BIN)/%.o : $(HAL)/Src/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
	
# Compile C source into object
hardware/%.o : hardware/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

# Compile C++ source into object
%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

# Compile assembler source into object
%.o : %.s
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

# Flash final elf into device
flash: $(ELF)
	${OPENOCD} -f board/stm32f4discovery-v2.1.cfg -c "program $< verify reset exit"
#	${OPENOCD} -f board/stm32f4discovery.cfg -c "program $< verify reset exit"

# Debug
debug: $(ELF)
	$(GDB) $(ELF) -ex "target remote | ${OPENOCD} -f board/stm32f4discovery-v2.1.cfg --pipe" -ex load
#	$(GDB) $(ELF) -ex "target remote | openocd -f board/stm32f4discovery.cfg --pipe" -ex load

-include $(DEPENDENCIES)
	
.PHONY: all flash clean debug
