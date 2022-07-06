#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!
#
##############################################################################################
#
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#
# Included originally in the yagarto projects. Original Author : Michael Fischer
# Modified to suit our purposes by Hussam Al-Hertani
# Use at your own risk!!!!!
##############################################################################################
# Start of default section
#
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary -S
MCU  = cortex-m0

# List all default C defines here, like -D_DEBUG=1
#para el micro STM32F051C8T6
# DDEFS = -DSTM32F051
#para el micro STM32F030K6T6
DDEFS = -DSTM32F030
#para el micro STM32G030J6M6
# DDEFS = -DSTM32G030xx

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR = ./src

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

#
# Define project name here
PROJECT        = Template_F030

# List C source files here
CORELIBDIR = ./cmsis_core
BOOTDIR = ./cmsis_boot

LINKER = ./cmsis_boot/startup

SRC  = ./src/main.c
SRC += $(BOOTDIR)/system_stm32f0xx.c
SRC += $(BOOTDIR)/syscalls/syscalls.c

SRC += ./src/adc.c
SRC += ./src/dsp.c
SRC += ./src/flash_program.c
SRC += ./src/gpio.c
SRC += ./src/hard.c
SRC += ./src/it.c
SRC += ./src/tim.c
# SRC += ./src/spi.c
SRC += ./src/uart.c
SRC += ./src/dma.c
SRC += ./src/sync.c
SRC += ./src/test_functions.c
SRC += ./src/gen_signal.c


## Core Support
SRC += $(CORELIBDIR)/core_cm0.c


## Other Peripherals libraries

# List ASM source files here
ASRC = ./cmsis_boot/startup/startup_stm32f0xx.s

# List User Directories for Libs Headers
UINCDIR = $(BOOTDIR) \
          $(CORELIBDIR)
	#../paho.mqtt.embedded-c/MQTTPacket/src

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

# Define optimisation level here
# O1 optimiza size no significativo
# O2 size mas fuerte
# Os (size mas fuerte que O2)
# O3 el mas fuerte, seguro despues no corre
# O0 (no optimiza casi nada, mejor para debug)
OPT = -O0

#
# End of user defines
##############################################################################################
#
# Define linker script file here
#
LDSCRIPT = $(LINKER)/stm32_flash.ld
FULL_PRJ = $(PROJECT)_rom

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

ADEFS   = $(DADEFS) $(UADEFS)

LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)

ASFLAGS = $(MCFLAGS) -g -gdwarf-2 -mthumb  -Wa,-amhls=$(<:.s=.lst) $(ADEFS)

# SIN INFO DEL DEBUGGER + STRIP CODE
# CPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -fdata-sections -ffunction-sections -fverbose-asm -Wa,-ahlms=$(<:.c=.lst)

# INFO PARA DEBUGGER + STRIP CODE + DUPLICATE GLOBALS ERROR
# CPFLAGS = $(MCFLAGS) $(OPT) -g -gdwarf-2 -fno-common -mthumb -fomit-frame-pointer -Wall -fdata-sections -ffunction-sections -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DDEFS)

# CON INFO PARA DEBUGGER + STRIP CODE
CPFLAGS = $(MCFLAGS) $(OPT) -g -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -fdata-sections -ffunction-sections -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DDEFS)

# SIN DEAD CODE, hace el STRIP
LDFLAGS = $(MCFLAGS) -mthumb -lm --specs=nano.specs -Wl,--gc-sections -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)
# CON DEAD CODE
#LDFLAGS = $(MCFLAGS) -mthumb --specs=nano.specs -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)
#LDFLAGS = $(MCFLAGS) -mthumb -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)

#
# makefile rules
#

assemblersources = $(ASRC)
sources = $(SRC)
OBJS  = $(SRC:.c=.o) $(ASRC:.s=.o)

objects = $(sources:.c=.o)
assobjects = $(assemblersources:.s=.o)

all: $(objects) $(assobjects) $(FULL_PRJ).elf $(FULL_PRJ).bin
	arm-none-eabi-size $(FULL_PRJ).elf
	gtags -q

$(objects): %.o: %.c
	$(CC) -c $(CPFLAGS) -I. $(INCDIR) $< -o $@

$(assobjects): %.o: %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

%hex: %elf
	$(HEX) $< $@

%bin: %elf
	$(BIN)  $< $@

flash:
	sudo openocd -f stm32f0_flash.cfg

flash_lock:
	sudo openocd -f stm32f0_flash_lock.cfg

gdb:
	sudo openocd -f stm32f0_gdb.cfg

reset:
	sudo openocd -f stm32f0_reset.cfg

clean:
	rm -f $(OBJS)
	rm -f $(FULL_PRJ).elf
	rm -f $(FULL_PRJ).map
	rm -f $(FULL_PRJ).hex
	rm -f $(FULL_PRJ).bin
	rm -f $(SRC:.c=.lst)
	rm -f $(SRC:.c=.su)
#   rm $(ASRC:.s=.s.bak)
	rm -f $(ASRC:.s=.lst)
	rm -f *.o
	rm -f *.out

tests:
	# simple functions tests, copy functions to test into main
	gcc src/tests.c
	./a.out

tests_signals:
	# tests on modules with no dependencies with hardware
	gcc -c src/dsp.c -I. $(INCDIR)
	gcc -c src/gen_signal.c -I. $(INCDIR)
	gcc -c src/tests_vector_utils.c -I. $(INCDIR)
	gcc src/tests_signals.c dsp.o gen_signal.o  tests_vector_utils.o -lm
	./a.out

tests_simul:
	# tests on modules with no dependencies with hardware
	gcc -c src/dsp.c -I. $(INCDIR)
	gcc -c src/tests_vector_utils.c -I. $(INCDIR)
	gcc -c src/tests_recursive_utils.c -I. $(INCDIR)
	gcc src/tests_simul.c dsp.o tests_vector_utils.o tests_recursive_utils.o -lm
	./a.out

tests_signals_simul:
	# tests on modules with no dependencies with hardware
	gcc -c src/dsp.c -I. $(INCDIR)
	gcc -c src/gen_signal.c -I. $(INCDIR)
	gcc -c src/tests_vector_utils.c -I. $(INCDIR)
	gcc -c src/tests_recursive_utils.c -I. $(INCDIR)
	gcc src/tests_signals_simul.c dsp.o gen_signal.o tests_vector_utils.o tests_recursive_utils.o -lm
	./a.out

tests_simul_dq:
	# first module objects to test
	gcc -g -c src/dsp.c -I. $(INCDIR)
	# gcc -g -c src/funcs_gsm.c -I $(INCDIR) $(DDEFS)
	# gcc -g -c src/sim900_800.c -I $(INCDIR) $(DDEFS)
	# gcc -g -c src/comm.c -I $(INCDIR) $(DDEFS)
	# second auxiliary helper modules
	gcc -g -c src/tests_ok.c -I $(INCDIR)
	gcc -g -c src/tests_vector_utils.c -I. $(INCDIR)
	gcc -g -c src/tests_recursive_utils.c -I. $(INCDIR)
	# linking
	gcc -g src/tests_simul_dq.c dsp.o tests_ok.o tests_vector_utils.o tests_recursive_utils.o -lm
	# test execution
	./a.out



# *** EOF ***
