# Builder applications
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f

#Main application file name
TARGET = main

#MCU specification
MCU = atxmega128a3u
F_CPU = 12000000UL

#Options for avr-gcc
OPTLEVEL = s
CDEFS := -DF_CPU=16000000UL
CDEFS += -DBOARD=STK600_RC064X
CFLAGS = $(CDEFS)
CFLAGS += -O$(OPTLEVEL)
CFLAGS += -mmcu=$(MCU)
CFLAGS += -std=gnu99
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wa,-adhlns=$(patsubst %.c,$(OUTDIR)/%.lst,$<)
#Linking options for avr-gcc
LFLAGS := -Os -mmcu=$(MCU)
LFLAGS += -Wl,--gc-sections
LFLAGS += -Wl,--print-gc-sections
LFLAGS += -Ilib/ASF/xmega/utils

#Options for HEX file generation
HFLAGS = -j .text -j .data -O ihex

#Options for avrdude to burn the hex file
#MMCU model here according to avrdude options
AVRDUDE_MCU = x128a3u
AVRDUDE_PROGRAMMER = dragon_pdi
AVRDUDE_FLAGS := -p $(AVRDUDE_MCU)
AVRDUDE_FLAGS += -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += -U flash:w:$(TARGET).hex:i
AVRDUDE_FLAGS += -v -e

SRCDIR = src
LIBDIR = lib
INCDIR = inc
OUTDIR = out

# Sources files needed for building the application 
SRCS := $(shell find $(SRCDIR) -name '*.c')
SRCS += $(shell find $(LIBDIR) -name '*.c')
INCS := $(addprefix -I,$(INCDIR))
INCS += $(addprefix -I,$(shell find $(LIBDIR) -type d))
OBJS := $(patsubst %.c,$(OUTDIR)/%.o,$(SRCS))
OBJS += $(shell find $(LIBDIR) -name '*.S')
LSTS := $(OBJS:.o=.lst)

MSG_LINKING = Linking...
MSG_COMPILING = Compiling...
MSG_FLASH = Preparing HEX file:
MSG_CLEAN = Cleaning project...
MSG_SHOW = Source files:

.PHONY: all build download clean size analyze show help
	
all: build size analyze download 

build: gccversion $(TARGET).hex

download: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS)

%.hex: %.elf
	@echo $(MSG_FLASH) $@
	@$(OBJCOPY) $(HFLAGS) $< $@

%.elf: $(OBJS)
	@echo $(MSG_LINKING) $@
	@$(CC) $(LFLAGS) $^ --output $(@F)

$(OUTDIR)/%.o : %.c
	@mkdir -p $(dir $@)
	@echo $(MSG_COMPILING) $<
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

gccversion:
	@$(CC) --version

size: $(TARGET).elf
	@echo ----------------------------------------
	$(SIZE) -C --mcu=$(MCU) $(TARGET).elf

analyze: $(TARGET).elf
	@echo ----------------------------------------
	$(NM) -S --size-sort -t decimal $(TARGET).elf

clean:
	@echo $(MSG_CLEAN)
	@rm -Rf $(OUTDIR)
	@rm -f $(TARGET).hex $(TARGET).elf

show:
	@echo Project name: $(TARGET)
	@echo MCU: $(MCU)
	@echo $(MSG_SHOW)
	@echo $(SRCS)

help:
	@echo Commands:
	@echo     show 		 - show project information
	@echo     gccversion - print version of avr-gcc
	@echo     build      - build the project
	@echo     download   - download the executable to the mcu
	@echo     size       - print code size
	@echo     analyze    - print code information
	@echo     all        - do gccversion, build, size, analyze and download 


