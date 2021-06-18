
# We require the avr-gcc toolchain
#
OBJDIR=obj

OBJ_FILES=main.o diag.o a7105_spi.o spi_asm.o radio.o motors.o nvconfig.o mixing.o weapons.o \
	vsense.o sticks.o motor_test.o
OBJECTS=$(addprefix $(OBJDIR)/,$(OBJ_FILES))
HEADERS=radio.h nvconfig.h state.h motors.h diag.h a7105_spi.h mixing.h weapons.h vsense.h\
	sticks.h motor_test.h

MCU=attiny817

ELF=$(OBJDIR)/main.elf
HEX=$(OBJDIR)/main.hex

# -flto is link-time optimisation and must be used for compile and link.
LINKFLAGS=-Os -flto

link: $(OBJECTS) $(HEADERS)
	avr-gcc -mmcu=$(MCU) -o $(ELF) $(OBJECTS) $(LINKFLAGS)
	# see how big it is
	avr-size $(ELF)
	# disassemble, so can see what the compiler did
	avr-objdump -S $(ELF) > $(OBJDIR)/main.asm
	avr-objdump -t $(ELF) > $(OBJDIR)/main.sym
	avr-objdump -j .bss -j .data -t $(ELF) | grep 00 |sort > $(OBJDIR)/ram.txt # show ram usage
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(ELF) $(HEX)
	avr-objcopy -j .text -j .data -j .rodata -O binary $(ELF) obj/main.bin

CFLAGS=-mmcu=$(MCU) -Os -Wall  -flto
$(OBJDIR)/%.o: %.c Makefile
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<

# Assembly language compile
$(OBJDIR)/%.o: %.S Makefile $(HEADERS)
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -Xassembler -a=$(OBJDIR)/$(<).lst -o $@ $<

install: link
	pyupdi -d tiny1617 -c /dev/ttyUSB0 -f $(HEX)
install1614: link
	pyupdi.py -d tiny1614 -c /dev/ttyUSB0 -f $(HEX)
install3217: link
	pyupdi.py -d tiny3217 -c /dev/ttyUSB0 -f $(HEX)
	
installpymcu: link
	pymcuprog -d attiny1617 -t uart -u /dev/ttyUSB0 -v info -f $(HEX) write

unbind:
	# Overwrite the magic number in eeprom so the receiver unbinds
	# and restores its settings to factory state.
	echo -n 'empty' > $(OBJDIR)/blank.bin
	pymcuprog -d attiny817 -t uart -u /dev/ttyUSB0 write -m eeprom -f $(OBJDIR)/blank.bin


motor_test:
	# Set fuse for motor test.
	pyupdi.py -d tiny1617 -c /dev/ttyUSB0 -fs 128:0x2a


clean:
	rm -r obj/
