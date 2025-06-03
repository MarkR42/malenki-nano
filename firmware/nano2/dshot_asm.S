#include <avr/io.h>

; Bit-bang dshot commands.
; See See https://brushlesswhoop.com/dshot-and-bidirectional-dshot/ 


; Port to use for dshot bit banging
; Define the vport to use to pulse sck line
; This is always set as an output.
vport_esc=VPORTA_OUT
bit_esc=3

.macro out_low ; dshot output low
    cbi vport_esc, bit_esc
.endm

.macro out_high ; dshot high
    sbi vport_esc, bit_esc
.endm

; bit-start-time should be 1.25us, or 12 cycles
; bit-hold-time should be 1.25us or 12 cycles
; remaining time should be about 8 cycles

.global dshot_send_word
dshot_send_word:
    ; Assume output is intially low.
    ; Assume called with interrupts disabled.

    ; By convention argument is in r24 and r25
    ; (see https://www.nongnu.org/avr-libc/user-manual/FAQ.html )
    ; r25 = high byte r24 = low byte
    ldi r18, 16 ; Loop counter variable
    ; Bits are sent most significant first.
dshot_loop:
    ; Send bit, time should be approx 32 cycles in total
    out_high
    ; Wait for approx 10 cycles
.rept 10
    nop
.endr
    ; Check the highest bit, we send highest first.
    sbrs r25, 7 ; Skip if the bit is SET, keep output high
    out_low ; Should be 12 cycles until unconditional out_low
    ; Shift r25:r24 left
    lsl r24 ;
    rol r25 ;
    
    ; 9 cycles left
.rept 9
    nop
.endr
    out_low ; Unconditional this time
    ; Approx 8 cycles left, dec & branch need 3 cycles
.rept 5
    nop
.endr
    
    
    ; Decrement counter and continue loop
    dec r18
    brne dshot_loop ; continue the loop
    
    ret
    

