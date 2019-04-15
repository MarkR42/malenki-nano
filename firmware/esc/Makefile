
# We require the toolchain avr-gcc described here:
# https://github.com/vladbelous/tinyAVR_gcc_setup
#
OBJDIR=obj

OBJ_FILES=main.o diag.o rxin.o motors.o vsense.o builddate.o nvconfig.o blinky.o mixing.o
OBJECTS=$(addprefix $(OBJDIR)/,$(OBJ_FILES))
HEADERS=diag.h motors.h rxin.h state.h vsense.h nvconfig.h blinky.h mixing.h

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
	avr-objdump -t $(ELF) |grep 00 |sort  > $(OBJDIR)/esc-sorted.sym
	avr-objdump -j .bss -j .data -t $(ELF) | grep 00 |sort > $(OBJDIR)/ram.txt # show ram usage
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(ELF) $(HEX)

CFLAGS=-mmcu=$(MCU) -D__AVR_DEV_LIB_NAME__=tn1614 -Os -Wall
$(OBJDIR)/%.o: %.c Makefile $(HEADERS)
	@mkdir -p $(OBJDIR)
	avr-gcc -c $(CFLAGS) -o $@ $<

builddate.c: .phony
	./gen-builddate.sh

install: link
	pyupdi.py -d tiny1614 -c /dev/ttyUSB0 -f $(HEX)

# Show the gcc preprocessor defines
show_defs:
	avr-gcc -mmcu=$(MCU) -dM -E - </dev/null 

# Dummy target:
.phony:
   
clean:
	rm -r obj/ builddate.c
