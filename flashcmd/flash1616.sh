#!/bin/bash

MCU=attiny1616
PYMCUPROG_OPTS="-d $MCU -t uart -u /dev/ttyUSB*"  
file=1616.bin

set -e
set -x
pymcuprog $PYMCUPROG_OPTS erase
pymcuprog $PYMCUPROG_OPTS -f $file -m flash -v info write 
pymcuprog $PYMCUPROG_OPTS -f $file -m flash verify

