
# We require the toolchain avr-gcc
# from ../../avr_toolchain
#
OBJDIR=obj

OBJ_FILES=main.o motors.o diag.o
OBJECTS=$(addprefix $(OBJDIR)/,$(OBJ_FILES))
HEADERS=motors.h diag.h state.h

MCU=attiny1616

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
	avr-objdump -j .bss -j .data -t $(ELF) | grep 00 |sort > $(OBJDIR)/ram.txt # show ram usage
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(ELF) $(HEX)

CFLAGS=-mmcu=$(MCU) -Os -Wall
$(OBJDIR)/%.o: %.c Makefile
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<

# Assembly language compile
$(OBJDIR)/%.o: %.S Makefile $(HEADERS)
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -Xassembler -a=$(OBJDIR)/$(<).lst -o $@ $<

PYMCUPROG_OPTS=-d attiny1616 -t uart -u /dev/ttyUSB0  

install: link
	pymcuprog $(PYMCUPROG_OPTS) erase
	pymcuprog $(PYMCUPROG_OPTS) -f $(HEX) -v info write 
	pymcuprog $(PYMCUPROG_OPTS) -f $(HEX) verify

clean:
	rm -r obj/
