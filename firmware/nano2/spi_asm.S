#include <avr/io.h>

; SPI clock line
; Define the vport to use to pulse sck line
; This is always set as an output.
vport_sck=VPORTA_OUT
bit_sck=7

; SPI chip-select line
; Define the vport to use to pulse scs line
; This is always set as an output.
vport_scs=VPORTB_OUT
bit_scs=3

; SPI data line
; Define the vport to use to set sdio dir 
vport_sdio_dir=VPORTA_DIR
; sdio output (data)
vport_sdio_out=VPORTA_OUT
vport_sdio_in=VPORTA_IN
; sdio bit index
bit_sdio=6

.macro scs_low ; chip select low - start of new spi transaction
    cbi vport_scs, bit_scs
.endm

.macro scs_high ; chip select high - end of spi transaction
    sbi vport_scs, bit_scs
.endm

.macro dir_out ; direction out - for writing on data line
    sbi vport_sdio_dir, bit_sdio
.endm
.macro dir_in ; direction in
    cbi vport_sdio_dir, bit_sdio
.endm
.macro data_high ; set data line
    sbi vport_sdio_out, bit_sdio
.endm
.macro data_low
    cbi vport_sdio_out, bit_sdio
.endm

.macro pulse_clock
    sbi vport_sck, bit_sck
    cbi vport_sck, bit_sck
.endm

; Send a byte on spi with the dir set out
; highest bit first.
; byte in r24
.macro send_byte
.rept 8
        sbrc r24,7 ; skip if bit 7 set
        data_high
        sbrs r24,7 ; skip if bit 7 set
        data_low
        pulse_clock
        lsl r24 
.endr
.endm

.macro send_nibble
.rept 4
        sbrc r24,7 ; skip if bit 7 set
        data_high
        sbrs r24,7 ; skip if bit 7 set
        data_low
        pulse_clock
        lsl r24 
.endr
.endm

; void spi_write_byte(uint8_t addr, uint8_t data)
.global spi_write_byte
spi_write_byte:
    ; First paramter is r24 (addr)
    ; Second param is r22 (data)
    scs_low
    dir_out
    send_byte ; r24 contains addr
    mov r24, r22 ; r24 now contains data
    send_byte
    dir_in
    scs_high
    ret

; void spi_strobe_then_write_block(uint8_t cmd,
;     uint8_t addr, uint8_t *buf, uint8_t datalen);

.global spi_strobe_then_write_block
spi_strobe_then_write_block:
    scs_low
    dir_out
    send_byte ; send strobe cmd in r24
    ; shift arguments into correct registers for spi_write_block
    mov r24, r22 ; addr
    movw r22, r20 ; buf
    mov r20, r18 ; datalen
    ; Fall through into spi_write_block

; void spi_write_block(uint8_t addr, const uint8_t *buf, uint8_t datalen)
.global spi_write_block
spi_write_block:
    ; r24 = addr
    ; r22:23 = buf
    ; r20 = datalen
    scs_low
    dir_out
    send_byte ; send addr
    movw X, r22 ; X pointer stores buf 
wbloop:
    tst r20 ; check if finished
    breq wbfinished
    ld r24, X+ ; next byte of data
    send_byte
    dec r20 ; decrement datalen
    rjmp wbloop
    
wbfinished:
    dir_in
    scs_high
    ret


.global spi_strobe
spi_strobe: ; command in r24
    scs_low
    dir_out
    ; According to the datasheet we can skip the next 4 bits.
    ; strobe commands only have the top 4 bits.
    send_nibble ; r24
    dir_in
    scs_high
    ret

; These two functions are actually identical, they just write
; three bytes to the SPI.
.global spi_strobe3
spi_strobe3: ; three commands in r24, r22, r20
.global spi_write_byte_then_strobe
spi_write_byte_then_strobe: ; addr: r24, data: r22, cmd: r20
    scs_low
    dir_out
    send_byte ; r24
    mov r24, r22
    send_byte ; r24
    mov r24, r20
    send_byte ; r24
    dir_in
    scs_high
    ret

; READ FUNCTIONS
; --------------

.macro read_byte_1
    ; assumes dir_in
    ; No need to clear r24, any previous data will be shifted out
.rept 8
    sbi vport_sck, bit_sck ; clock high
    lsl r24 ; r24 << 1
    sbic vport_sdio_in, bit_sdio    
    ori r24, 1 ; ; set lowest bit in r24
    cbi vport_sck, bit_sck ; clock low
.endr
.endm

; uint8_t spi_read_byte(uint8_t addr) {
; Read a single byte
.global spi_read_byte
spi_read_byte:
    ; addr in r24, return in r24
    ori r24, 0x40 ; Set read-bit in address
    scs_low
    dir_out
    send_byte ; r24
    dir_in
    read_byte_1
    scs_high
    ret

.global spi_strobe_then_read_block
spi_strobe_then_read_block:
; spi_strobe_then_read_block(uint8_t cmd, uint8_t addr, uint8_t *buf, uint8_t datalen)
    ; r24= cmd, addr = r22, *buf = r20:r21, datalen=r18
    scs_low
    dir_out
    send_byte ; send strobe cmd in r24
    ; shift arguments into correct registers for spi_read_block
    mov r24, r22 ; addr
    movw r22, r20 ; buf
    mov r20, r18 ; datalen
    ; Fall through into spi_read_block
 
; void spi_read_block(uint8_t addr, uint8_t *buf, uint8_t datalen) {
.global spi_read_block
spi_read_block:
    ; r24 = addr, r22:r23 = *buf, r20=datalen
    ori r24, 0x40 ; Set read-bit in address
    scs_low
    dir_out
    send_byte ; r24
    dir_in
    movw X, r22 ; Use X register as pointer. 
rbloop:
    read_byte_1
    st X+, r24
    dec r20
    brne rbloop

    scs_high
    ret
