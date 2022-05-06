#!/bin/bash

set -x
java -jar /home/mark/unpack/kicad-util/target/kicadutil-1.0-SNAPSHOT.jar \
    pcb -f panel2-1616.kicad_pcb panel --inset=-0.1 --width=2.0 --pitch 1.2
