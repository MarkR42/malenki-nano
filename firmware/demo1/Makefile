
# We require the toolchain avr-gcc described here:
# https://github.com/vladbelous/tinyAVR_gcc_setup
#
OBJDIR=obj

OBJECTS=$(OBJDIR)/main.o $(OBJDIR)/diag.o
HEADERS=diag.h

MCU=attiny1614

ELF=$(OBJDIR)/main.elf
HEX=$(OBJDIR)/main.hex

LINKFLAGS=-Os

link: $(OBJECTS) $(HEADERS)
	avr-gcc -mmcu=$(MCU) -o $(ELF) $(OBJECTS) $(LINKFLAGS)
	# see how big it is
	avr-size $(ELF)
	# disassemble, so can see what the compiler did
	avr-objdump -S $(ELF) > $(OBJDIR)/main.asm
	avr-objdump -t $(ELF) > $(OBJDIR)/main.sym
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(ELF) $(HEX)

CFLAGS=-mmcu=$(MCU) -D__AVR_DEV_LIB_NAME__=tn1614 -Os -Wall
$(OBJDIR)/%.o: %.c Makefile
	mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<
		
install:
	pyupdi.py -d tiny1614 -c /dev/ttyUSB0 -f $(HEX)

clean:
	rm -r obj/
