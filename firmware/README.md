This directory will contain the firmware (embedded software) for 
our microcontroller (attiny1614 at time of writing)

Required to compile
-------------------

To compile these, you need a avr-gcc compiler configured for the
attiny1614.

Instructions are here:
https://github.com/vladbelous/tinyAVR_gcc_setup

To flash the firmware, you need a suitable flashing utility.

pyupdi can be used with a simple serial cable + 1 resistor.

See https://github.com/mraardvark/pyupdi/

Useful examples:

https://github.com/chromia/attiny1614example

