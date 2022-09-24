
# We require the avr-gcc toolchain
#
OBJDIR=obj_$(MCU)$(PRODUCT_MODEL)

OBJ_FILES=main.o diag.o a7105_spi.o spi_asm.o radio.o motors.o nvconfig.o mixing.o weapons.o \
	vsense.o sticks.o
OBJECTS=$(addprefix $(OBJDIR)/,$(OBJ_FILES))
HEADERS=radio.h nvconfig.h state.h motors.h diag.h a7105_spi.h mixing.h weapons.h vsense.h\
	sticks.h

MAKEFILES=Makefile.817 common.mk

ELF=$(OBJDIR)/main.elf
HEX=$(OBJDIR)/main.hex

LTOFLAGS=-flto
# -flto is link-time optimisation and must be used for compile and link.
LINKFLAGS=-g -Os $(LTOFLAGS)

link: $(OBJECTS) $(HEADERS)
	avr-gcc -mmcu=$(MCU) -o $(ELF) $(OBJECTS) $(LINKFLAGS)
	# see how big it is
	avr-size $(ELF)
	# disassemble, so can see what the compiler did
	avr-objdump --source --source-comment -S $(ELF) > $(OBJDIR)/main.asm
	avr-objdump -t $(ELF) > $(OBJDIR)/main.sym
	avr-objdump -j .bss -j .data -t $(ELF) | grep 00 |sort > $(OBJDIR)/ram.txt # show ram usage
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(ELF) $(HEX)
	avr-objcopy -j .text -j .data -j .rodata -O binary $(ELF) $(OBJDIR)/main.bin

CFLAGS += -mmcu=$(MCU) -Os -Wall -g  $(LTOFLAGS)
$(OBJDIR)/%.o: %.c $(MAKEFILES) $(HEADERS)
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<

# Assembly language compile
$(OBJDIR)/%.o: %.S $(MAKEFILES) $(HEADERS)
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -Xassembler -a=$(OBJDIR)/$(<).lst -o $@ $<

install: installpymcu
	
PYMCUPROG_OPTS=-d $(MCU_PYMCUPROG) -t uart -u /dev/ttyUSB0  
	
installpymcu:
	pymcuprog $(PYMCUPROG_OPTS) erase
	pymcuprog $(PYMCUPROG_OPTS) -f $(HEX) -v info write 
	pymcuprog $(PYMCUPROG_OPTS) -f $(HEX) verify
	
unbind:
	# Overwrite the magic number in eeprom so the receiver unbinds
	# and restores its settings to factory state.
	pymcuprog $(PYMCUPROG_OPTS) -m eeprom erase

reset:
	pymcuprog $(PYMCUPROG_OPTS) reset

clean:
	rm -rf $(OBJDIR)
