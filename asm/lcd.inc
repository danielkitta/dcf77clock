; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; LCD routines
;

; LCD control words.
;
.LCD_8BIT:      equ     00111000b               ; init display in 8-bit mode

; Reset LCD.
; Input  : -
; Output : -
; Scratch: af, bc, hl
;
resetlcd:       ld      a, .LCD_8BIT            ; dummy 2nd nibble of possibly
                out     (LCDCW), a              ; interrupted 4-bit command

                call    halt4100us              ; wait max duration of any cmd

                ld      a, .LCD_8BIT            ; switch to 8-bit mode
                out     (LCDCW), a              ; (may still be in 4-bit mode)

                call    halt100us               ; wait for known cmd to finish

                ld      a, .LCD_8BIT            ; dummy 2nd nibble of command
                out     (LCDCW), a              ; if still in 4-bit mode

                ld      bc, LCDINITLEN*100h+LCDCW
                ld      hl, lcdinitseq          ; continue with regular 8-bit
                call    .writetextloop          ; initialization sequence

                ld      b, LCDCGDATALEN         ; follow up with special
                call    writelcdtext            ; character graphics data

                ld      c, LCDPOSSTAT           ; set cursor
                call    writelcdctrl
                ld      c, BITPULSE             ; show radio antenna symbol
                jp      writelcddata

; Look up and write a sequence of four bytes to the LCD data register.
; Input  : a, hl
; Output : -
; Scratch: af, bc, hl
;
writelcdtable4: add     a, a                    ; quadruple index
                add     a, a
                ld      b, 0                    ; no carry: max index 63
                ld      c, a
                add     hl, bc                  ; add to base address
                ld      b, 4                    ; string length 4

; Write a sequence of bytes to the LCD data register.
; Input  : b, hl
; Output : -
; Scratch: af, bc, hl
;
writelcdtext:   ld      c, LCDDW
.writetextloop: in      a, (LCDCR)              ; read busy flag
                rlca
                jr      c, .writetextloop       ; loop as long bit 7 is set
                outi                            ; out (c), (hl++); --b
                ret     z                       ; return if b=0
                jp      .writetextloop          ; otherwise repeat

; Write a byte to the LCD control register.
; Input  : c
; Output : -
; Scratch: af
;
writelcdctrl:   in      a, (LCDCR)              ; read busy flag
                rlca
                jr      c, writelcdctrl         ; loop as long bit 7 is set
                ld      a, c
                out     (LCDCW), a              ; output control word
                ret

; Write a 2-digit packed BCD number to the LCD data register,
; while suppressing a possibly leading zero digit.
; Input  : a
; Output : -
; Scratch: af, bc
;
writelcdbcdslz: cp      10h                     ; BCD number below 10?
                jr      nc, writelcdbcd         ; no: normal BCD output

                or      '0'                     ; digit to ASCII
                ld      b, a                    ; save argument
                ld      c, ' '
                call    writelcddata            ; write space
                ld      c, b
                jp      writelcddata            ; write second digit

; Write a 2-digit packed BCD number to the LCD data register.
; Input  : a
; Output : -
; Scratch: af, bc
;
writelcdbcd:    ld      b, a                    ; save argument
                rrca                            ; rotate first digit
                rrca                            ; to lower nibble
                rrca
                rrca
                call    .writelcdbcd1           ; write first digit
                ld      a, b                    ; restore argument

.writelcdbcd1:  and     0Fh                     ; isolate lower nibble
                or      '0'                     ; digit to ASCII
                ld      c, a

; Write a byte to the LCD data register.
; Input  : c
; Output : -
; Scratch: af
;
writelcddata:   in      a, (LCDCR)              ; read busy flag
                rlca
                jr      c, writelcddata         ; loop as long bit 7 is set
                ld      a, c
                out     (LCDDW), a              ; output data byte
                ret
