; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Bit stream decoding
;

; Bit masks corresponding to status bits.
;
.DCFSYNCMASK:   equ     1<<DCFSYNC
.DCFERRORMASK:  equ     1<<DCFERROR

; Decode DCF77 time stamp.
; Input  : iy
; Output : cy
; Scratch: af, bc, de, hl
;
dcfdecode:      ld      (iy+I_VALID), 0         ; reset valid flag

                ld      a, (STATUS)
                and     .DCFSYNCMASK|.DCFERRORMASK
                xor     .DCFSYNCMASK            ; sync set, error unset?
                scf
                ret     nz                      ; no: error

                ld      a, (BITCOUNT)           ; number of received bits
                sub     59                      ; less than 59 bytes?
                ret     c                       ; yes: error
                ld      d, a                    ; remember remaining bytes

                ld      a, (DCFBITS+0)
                add     a, -1                   ; bit 0 is zero?
                ret     c                       ; no: return with error

                ld      hl, DCFBITS+17          ; time zone bits
                ld      a, (hl)
                rlca
                inc     hl
                or      (hl)
                ld      (iy+I_TIMEZONE), a
                add     a, -3                   ; time zone offset < 3?
                ret     c                       ; no: return with error

                ld      a, (DCFBITS+20)
                cp      1                       ; bit 20 is set?
                ret     c                       ; no: return with error

                ld      hl, DCFBITS+58
                ld      e, (hl)                 ; read date parity

                ld      b, 8
                call    .readbits               ; read year digits
                rl      e                       ; store parity
                cp      99h+1                   ; check year range 0..99
                ccf
                ret     c
                ld      (iy+I_YEAR), a
                and     0Fh                     ; isolate lower digit
                add     a, ~09h                 ; range 0..9?
                ret     c

                ld      b, 5
                call    .readbits               ; read year digits
                rl      e                       ; store parity
                cp      1                       ; month >= 1?
                ret     c
                cp      12h+1                   ; month <= 12?
                ccf
                ret     c
                ld      (iy+I_MONTH), a
                and     0Fh                     ; isolate lower digit
                add     a, ~09h                 ; range 0..9?
                ret     c

                ld      b, 3
                call    .readbits               ; read day of week number
                rl      e                       ; store parity
                ld      (iy+I_WEEKDAY), a
                dec     a
                add     a, ~06h                 ; range 1..7?
                ret     c

                ld      b, 6
                call    .readbits               ; read day of month
                rl      e                       ; store parity
                jp      po, .reterror           ; odd parity: error
                cp      1                       ; day >= 1?
                ret     c
                cp      31h+1                   ; day <= 12?
                jr      nc, .reterror
                ld      (iy+I_DAY), a
                and     0Fh                     ; isolate lower digit
                add     a, ~09h                 ; range 0..9?
                ret     c

                ld      b, 7
                call    .readbits               ; read hour
                ret     c                       ; odd parity: error
                and     3Fh                     ; remove parity bit
                cp      23h+1                   ; range 0..23?
                jr      nc, .reterror
                ld      (iy+I_HOUR), a
                and     0Fh                     ; isolate lower digit
                add     a, ~09h                 ; range 0..9?
                ret     c

                ld      b, 8
                call    .readbits               ; read minute
                ret     c                       ; odd parity: error
                and     7Fh                     ; remove parity bit
                cp      59h+1                   ; range 0..59?
                jr      nc, .reterror
                ld      e, a                    ; remember minute
                and     0Fh                     ; isolate lower digit
                add     a, ~09h                 ; range 0..9?
                ret     c

                ld      (iy+I_MIN), e
                xor     a
                cp      e                       ; cy=1 if minute > 0
                sbc     a, a                    ; a=-cy, cy unchanged
                ld      c, a                    ; remember mask

                ld      a, (DCFBITS+19)         ; read leap second announcement
                ld      e, a                    ; remember flag
                add     a, c                    ; a=1 if leap=1 and minute=0
                dec     a                       ; a=1?
                jr      nz, .skipleapbit

                ld      a, (DCFBITS+59)         ; read leap second fill bit
                dec     d                       ; eat extra bit from count
                add     a, -1                   ; fill bit is 0?
                ret     c                       ; no: error

.skipleapbit:   xor     a
                cp      d                       ; remaining bit count is 0?
                ret     c                       ; no: error

                ld      a, (DCFBITS+16)         ; read DST switch announcement
                rlca                            ; move to bit 1
                or      e                       ; combine with leap second flag
                and     c                       ; clear flags if minute 0, cy=0
                ld      (iy+I_DISCONT), a       ; store discontinuity flags

                ld      (iy+I_VALID), 1         ; tag record as valid
                ret                             ; cy=0 from above

; Accumulate bits from memory, moving backwards.
; Input  : hl, b
; Output : a, hl, cy
; Scratch: f, b
;
.readbits:      xor     a                       ; a=0

.nextbit:       dec     hl
                rlca
                or      (hl)                    ; read bit
                djnz    .nextbit

                ret     pe                      ; even parity: return cy=0

.reterror:      scf                             ; odd parity: return cy=1
                ret
