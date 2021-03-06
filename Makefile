PROJECT_NAME=mechatronics

BUILDDIR = build

DEVICE = inc/STM32F4xx
CORE = inc/CMSIS
PERIPH = inc/STM32F4xx_StdPeriph_Driver
DISCOVERY = inc
USB = inc/usb

#SOURCES += $(DISCOVERY)/src/stm32f4_discovery.c

SOURCES += \
			$(PERIPH)/src/stm32f4xx_gpio.c \
			$(PERIPH)/src/stm32f4xx_i2c.c \
			$(PERIPH)/src/stm32f4xx_rcc.c \
			$(PERIPH)/src/stm32f4xx_spi.c \
			$(PERIPH)/src/stm32f4xx_exti.c \
			$(PERIPH)/src/stm32f4xx_syscfg.c \
			$(PERIPH)/src/stm32f4xx_tim.c \
			$(PERIPH)/src/stm32f4xx_usart.c \
			$(PERIPH)/src/stm32f4xx_dma.c \
			$(PERIPH)/src/stm32f4xx_adc.c \
			$(PERIPH)/src/misc.c

SOURCES += startup_stm32f4xx.S
#SOURCES += stm32f4xx_it.c
SOURCES += system_stm32f4xx.c

SOURCES += \
		src/utils.c \
		src/main.c \
		src/usart.c \
		src/accel.c \
		src/colorSensor.c \
		src/servo.c \
		src/motors.c \
		src/localize.c \
		src/map.c \
		src/state.c \
		src/pid.c 

OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I$(DEVICE) \
			-I$(CORE) \
			-I$(PERIPH)/inc \
			-I$(DISCOVERY) \
			-I$(USB)/inc \
			-I.

ELF = $(BUILDDIR)/$(PROJECT_NAME).elf
HEX = $(BUILDDIR)/$(PROJECT_NAME).hex
BIN = $(BUILDDIR)/$(PROJECT_NAME).bin

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
GDB = arm-none-eabi-gdb

CFLAGS  = -O0 -g -Wall -I. -std=c99\
   -mcpu=cortex-m4 -mthumb \
   -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
   $(INCLUDES) -DUSE_STDPERIPH_DRIVER

LDSCRIPT = stm32_flash.ld
LDFLAGS += -T$(LDSCRIPT) -nostdlib -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -lm

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

flash: $(BIN)
	qstlink2 --cli --erase --write $(BIN)

debug: $(ELF)
	$(GDB) -tui $(ELF)

all: $(HEX) $(BIN)

.PHONY: clean
clean:
	rm -rf build
