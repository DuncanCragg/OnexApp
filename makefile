
############################################################################################
# makefile for OnexAppIoT
############################################################################################

targets:
	@grep '^[a-zA-Z0-9\.#-]\+:' makefile | grep -v '^\.' | grep -v targets | sed 's/:.*//' | uniq | sed 's/\.elf/.hex/' | sed 's/^/make clean /'

############################################################################################

NRF5_INCLUDES = \
-I./OnexKernel/include \
-I./OnexLang/include \
-I./OnexKernel/src/ \
-I./OnexKernel/src/onp/ \
-I./OnexKernel/src/platforms/nrf5/ \


NRF5_C_SOURCE_FILES = \
./OnexKernel/src/platforms/nrf5/properties.c \
./OnexKernel/src/platforms/nrf5/serial.c \
./OnexKernel/src/platforms/nrf5/channel-serial.c \
./OnexKernel/src/platforms/nrf5/log.c \
./OnexKernel/src/platforms/nrf5/gpio.c \
./OnexKernel/src/platforms/nrf5/time.c \
./OnexKernel/src/platforms/nrf5/random.c \
./OnexKernel/src/platforms/nrf5/radio.c \


NRF51_SYS_S_OBJECTS = \
./OnexKernel/src/platforms/nrf5/gcc_startup_nrf51.s \


NRF52_SYS_S_OBJECTS = \
./OnexKernel/src/platforms/nrf5/gcc_startup_nrf52.S \


NRF51_SYS_C_OBJECTS = \
./OnexKernel/src/platforms/nrf5/system_nrf51.c \
./OnexKernel/src/platforms/nrf5/syscalls.c \


NRF52_SYS_C_OBJECTS = \
./OnexKernel/src/platforms/nrf5/system_nrf52.c \
./OnexKernel/src/platforms/nrf5/syscalls.c \


LIB_OBJECTS = \
./OnexKernel/src/lib/list.c \
./OnexKernel/src/lib/value.c \
./OnexKernel/src/onp/onp.c \
./OnexKernel/src/onf/onf.c \
./OnexLang/src/behaviours.c \


ONEXAPP_IOT_OBJECTS = \
./src/OnexAppIoT.c \


############################################################################################

onexapp.microbit.elf: COMPILE_LINE=${M0_CPU} $(M0_CC_FLAGS) $(NRF51_CC_SYMBOLS) $(NRF5_INCLUDES)
onexapp.microbit.elf: TARGET=TARGET_MICRO_BIT
onexapp.microbit.elf: CHANNELS=-DONP_CHANNEL_SERIAL
onexapp.microbit.elf: $(NRF51_SYS_S_OBJECTS:.s=.o) $(NRF51_SYS_C_OBJECTS:.c=.o) $(NRF5_C_SOURCE_FILES:.c=.o) ${LIB_OBJECTS:.c=.o} $(ONEXAPP_IOT_OBJECTS:.c=.o)
	$(LD) $(M0_LD_FLAGS) -L${M0_TEMPLATE_PATH} -T$(LINKER_SCRIPT_16K) -o $@ $^

#############################:

microbit.onexapp: onexapp.microbit.hex
	cp $< /media/duncan/MICROBIT/

############################################################################################

CC_FLAGS = -c -std=gnu99 -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer

M4_CPU = -mcpu=cortex-m4 -mthumb -mabi=aapcs
M4_CC_FLAGS = -std=c99 -MP -MD -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fshort-enums -fno-builtin -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -O3 -g3 -mfloat-abi=hard -mfpu=fpv4-sp-d16
NRF52_CC_SYMBOLS = -DNRF5 -DNRF52 -D${TARGET} ${CHANNELS} -DTARGET_MCU_NRF52832 -DFLOAT_ABI_HARD -DNRF52840_XXAA -D__HEAP_SIZE=8192 -D__STACK_SIZE=8192

M0_CPU = -mcpu=cortex-m0 -mthumb
M0_CC_FLAGS = -std=gnu99 -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -O0
NRF51_CC_SYMBOLS = -DNRF5 -DNRF51 -D${TARGET} ${CHANNELS} -DTARGET_MCU_NRF51822

M0_LD_FLAGS = $(M0_CPU) -O0 --specs=nano.specs

M4_LD_FLAGS = $(M4_CPU) -O3 -g3 -mthumb -mabi=aapcs -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs

M0_TEMPLATE_PATH := ./OnexKernel/src/platforms/nrf5/

M4_TEMPLATE_PATH := ./OnexKernel/src/platforms/nrf5/

LINKER_SCRIPT_16K=./OnexKernel/src/platforms/nrf5/memory-16K-no-sd.ld

LINKER_SCRIPT_256K=./OnexKernel/src/platforms/nrf5/memory-256K-no-sd.ld

############################################################################################

GNU_INSTALL_ROOT := /home/duncan/gcc-arm
GCC_BIN=$(GNU_INSTALL_ROOT)/bin

CC      = $(GCC_BIN)/arm-none-eabi-gcc
LD      = $(GCC_BIN)/arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)/arm-none-eabi-objcopy
SIZE    = $(GCC_BIN)/arm-none-eabi-size

############################################################################################

.s.o:
	$(CC) $(M0_CPU) -c -x assembler-with-cpp -D__STACK_SIZE=4096 -D__HEAP_SIZE=8192 -o $@ $<

.c.o:
	$(CC) ${COMPILE_LINE} -o $@ -c $<

%.hex: %.elf
	$(SIZE) "$<"
	$(OBJCOPY) -O ihex $< $@

#############################:

clean:
	find src tests -name '*.o' -o -name '*.d' | xargs rm -f
	rm -f *.bin *.elf
	rm -f ,*
	(cd OnexKernel; make clean)
	(cd OnexLang; make clean)
	@echo "------------------------------"

cleanx: clean
	rm -f *.hex

############################################################################################

