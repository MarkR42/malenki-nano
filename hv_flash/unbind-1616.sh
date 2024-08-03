#!/bin/bash

MCU=attiny1616
PYMCUPROG_OPTS="-d $MCU -t uart -u /dev/ttyUSB*"  
file=817.bin

set -e
set -x
pymcuprog $PYMCUPROG_OPTS -m eeprom erase

