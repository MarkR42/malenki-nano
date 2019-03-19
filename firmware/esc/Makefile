
# We require the toolchain avr-gcc described here:
# https://github.com/vladbelous/tinyAVR_gcc_setup
#
OBJDIR=obj

OBJECTS=$(OBJDIR)/main.o $(OBJDIR)/diag.o $(OBJDIR)/rxin.o $(OBJDIR)/motors.o $(OBJDIR)/vsense.o $(OBJDIR)/builddate.o
HEADERS=diag.h motors.h rxin.h state.h

MCU=attiny1614

ELF=$(OBJDIR)/esc.elf
HEX=$(OBJDIR)/esc.hex

LINKFLAGS=-Os

link: $(OBJECTS) $(HEADERS)
	avr-gcc -mmcu=$(MCU) -o $(ELF) $(OBJECTS) $(LINKFLAGS)
	# see how big it is
	avr-size $(ELF)
	# disassemble, so can see what the compiler did
	avr-objdump -S $(ELF) > $(OBJDIR)/esc.asm
	avr-objdump -t $(ELF) > $(OBJDIR)/esc.sym
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(ELF) $(HEX)

CFLAGS=-mmcu=$(MCU) -D__AVR_DEV_LIB_NAME__=tn1614 -Os -Wall
$(OBJDIR)/%.o: %.c Makefile
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<

builddate.c: .phony
	./gen-builddate.sh

install: link
	pyupdi.py -d tiny1614 -c /dev/ttyUSB0 -f $(HEX)

# Dummy target:
.phony:
   
clean:
	rm -r obj/ builddate.c
