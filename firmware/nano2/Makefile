
# We require the avr-gcc toolchain
#
OBJDIR=obj

OBJ_FILES=main.o diag.o a7105_spi.o radio.o motors.o nvconfig.o mixing.o zlast.o
OBJECTS=$(addprefix $(OBJDIR)/,$(OBJ_FILES))
HEADERS=radio.h nvconfig.h state.h motors.h diag.h a7105_spi.h mixing.h

MCU=attiny3217

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
	avr-objcopy -j .text -j .data -j .rodata -O binary $(ELF) obj/main.bin

CFLAGS=-mmcu=$(MCU) -Os -Wall
$(OBJDIR)/%.o: %.c Makefile
	mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<
		
install: link
	pyupdi.py -d tiny3217 -c /dev/ttyUSB0 -f $(HEX)
install1614: link
	pyupdi.py -d tiny1614 -c /dev/ttyUSB0 -f $(HEX)



clean:
	rm -r obj/
