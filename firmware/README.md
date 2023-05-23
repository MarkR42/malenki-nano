This directory will contain the firmware (embedded software) for 
our microcontroller (attiny1616 at time of writing)

Required to compile
-------------------

To compile these, you need a avr-gcc compiler configured for the
attiny1616.

Toolchain here:
https://github.com/MarkR42/avr_toolchain

The main firmware is in nano2/ directory.

Compile using the commands

make -f Makefile.1616  # For the Malenki-Nano
make -f Makefile.plus1616 # For the Malenki-HV (High Voltage) variant

Other Makefiles are for older versions of the board which used different
MCU chips during the chip shortage in 2021

To flash the firmware, you need a suitable flashing utility.

pyupdi can be used with a simple serial cable + 1 resistor.

See https://github.com/mraardvark/pyupdi/

Useful examples:

https://github.com/chromia/attiny1614example

