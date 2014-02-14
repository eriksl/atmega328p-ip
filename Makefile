# BOARD = 0		testboard 1

BOARD =	0

ifeq ($(BOARD), 0)
	USE_EXT_CRYSTAL	= 0
	MCUSPEED	= 8000000
	LFUSE		= 0xe2
	#HFUSE		= 0xd7
	HFUSE		= 0x97
endif

MCU			=		atmega328p
PROGRAMMER	=		dragon_isp
PRGFLAGS	=		-P usb -B 10

PROGRAM		=		main
OBJFILES	=		$(PROGRAM).o watchdog.o spi.o enc.o net.o ethernet.o arp.o ipv4.o icmp4.o tcp4.o content.o stats.o util.o
HEADERS		=		             watchdog.h spi.h enc.h net.h ethernet.h arp.h ipv4.h icmp4.h tcp4.h content.h stats.h util.h
HEXFILE		=		$(PROGRAM).hex
ELFFILE		=		$(PROGRAM).elf
PROGRAMMED	=		.programmed
CFLAGS		=		-I$(CURDIR) \
					-g --std=c99 -Wall -Winline -O0 -mmcu=$(MCU) -DF_CPU=$(MCUSPEED) -DUSE_EXT_CRYSTAL=$(USE_EXT_CRYSTAL) -DBOARD=$(BOARD) \
					-fpack-struct -fno-keep-static-consts -frename-registers -Wno-unused-variable
LDFLAGS		=		-Wall -mmcu=$(MCU)

.PHONY:				all clean hex
.SUFFIXES:
.SUFFIXES:			.c .o .elf .hex
.PRECIOUS:			.c .h

all:				$(PROGRAMMED)
hex:				$(HEXFILE)

fuse:
					avrdude -v -c $(PROGRAMMER) -p $(MCU) $(PRGFLAGS) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

$(PROGRAM).o:		$(PROGRAM).c $(HEADERS)

watchdog.o:			watchdog.h
spi.o:				$(HEADERS)
enc.o:				$(HEADERS) enc-private.h
net.o:				$(HEADERS)
ethernet.o:			$(HEADERS)
arp.o:				$(HEADERS)
ipv4.o:				$(HEADERS)
icmp4.o:			$(HEADERS)
tcp4.o:				$(HEADERS)
content.o:			content.h
stats.o:			stats.h

watchdog.o:			watchdog.c
					@echo "CC(S) $< -> $@"
					@avr-gcc -c $(CFLAGS) -O2 $< -o $@

%.o:				%.c
					@echo "CC $< -> $@"
					@avr-gcc -c $(CFLAGS) $< -o $@

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
