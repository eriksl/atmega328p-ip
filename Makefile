MCUSPEED	= 11059200
LFUSE		= 0xf7
# no DebugWire
#HFUSE		= 0xd7
# DebugWire
HFUSE		= 0x97
#CCOPTFLAGS	= -O0 -g
#CCOPTFLAGS	= -O3
#CCOPTFLAGS	= -O3
CCOPTFLAGS	= -Os

MCU			=		atmega328p
PROGRAMMER	=		dragon_isp
PRGFLAGS	=		-P usb -B 5 -y -V

PROGRAM		=		main
OBJFILES	=		$(PROGRAM).o twi_master.o esp.o uart.o \
						application.o application-sensor.o application-twi.o application-timer.o \
						stats.o util.o eeprom.o stackmonitor.o sensor.o
HEADERS		=			application.h stats.h util.h eeprom.h stackmonitor.h util.h
HEXFILE		=		$(PROGRAM).hex
ELFFILE		=		$(PROGRAM).elf
PROGRAMMED	=		.programmed
CFLAGS		=		-I$(CURDIR) \
					--std=gnu99 -Wall -Winline $(CCOPTFLAGS) -mmcu=$(MCU) -DF_CPU=$(MCUSPEED) \
					-fpack-struct -fno-keep-static-consts -frename-registers \
					-fdata-sections -ffunction-sections
LDFLAGS		=		-Wall -mmcu=$(MCU) -Wl,-gc-sections -Wl,-u,vfprintf -lprintf_flt -lm

.PHONY:				all clean hex
.SUFFIXES:
.SUFFIXES:			.c .o .elf .hex
.PRECIOUS:			.c .h

all:				$(PROGRAMMED)
hex:				$(HEXFILE)

fuse:
					avrdude -v -c $(PROGRAMMER) -p $(MCU) $(PRGFLAGS) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

info:
					avrdude -v -c $(PROGRAMMER) -p $(MCU) $(PRGFLAGS) -v

$(PROGRAM).o:		$(PROGRAM).c $(HEADERS)

twi_master.o:		twi_master.h
esp.o:				esp.h
uart.o:				uart.h
stats.o:			stats.h
application.o:		$(HEADERS) application-timer.h application-sensor.h application-twi.h
sensor.o:			sensor.h
eeprom.o:			eeprom.h

%.o:				%.c
					@echo "CC $< -> $@"
					@avr-gcc -c $(CFLAGS) $< -o $@

%.i:				%.c
					@echo "CC $< -> $@"
					@avr-gcc -E -c $(CFLAGS) $< -o $@

%.o:				%.S
					@echo "AS $< -> $@"
					@avr-gcc -x assembler-with-cpp -c $(CFLAGS) $< -o $@

%.s:				%.c
					@echo "CC (ASM) $< -> $@"
					@avr-gcc -S $(CFLAGS) $< -o $@

$(ELFFILE):			$(OBJFILES)
					@echo "LD $(OBJFILES) -> $@"
					@avr-gcc $(LDFLAGS) $(OBJFILES) -o $@

$(HEXFILE):			$(ELFFILE)
					@echo "OBJCOPY $< -> $@"
					@avr-objcopy -j .text -j .data -O ihex $< $@
					@sh -c 'avr-size $< | (read header; read text data bss junk; echo "SIZE: flash: $$[text + data] ram: $$[data + bss]")'

$(PROGRAMMED):		$(HEXFILE)
					@echo "AVRDUDE $^"
					@avrdude -c $(PROGRAMMER) -p $(MCU) $(PRGFLAGS) -U flash:w:$^

clean:			
					@echo "RM $(OBJFILES) $(ELFFILE) $(HEXFILE) $(PROGRAMMED)"
					@-rm $(OBJFILES) $(ELFFILE) $(HEXFILE) 2> /dev/null || true
