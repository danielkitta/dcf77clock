; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Constant data segment
;

                defs    -$&0Fh, 0       ; align to 16-byte boundary

; Name lookup table for the day of the week.
;
daynames:       defm    'Mon,'
                defm    'Tue,'
                defm    'Wed,'
                defm    'Thu,'
                defm    'Fri,'
                defm    'Sat,'
                defm    'Sun,'

; Name lookup table for the month.
;
monthnames:     defm    ' Jan'
                defm    ' Feb'
                defm    ' Mar'
                defm    ' Apr'
                defm    ' May'
                defm    ' Jun'
                defm    ' Jul'
                defm    ' Aug'
                defm    ' Sep'

; Lookup table for the number of days per month.
;
dayspermonth:   defb    31h             ; January
                defb    28h             ; February
                defb    31h             ; March
                defb    30h             ; April
                defb    31h             ; May
                defb    30h             ; June
                defb    31h             ; July
                defb    31h             ; August
                defb    30h             ; September
                defs    6
                defb    31h             ; October
                defb    30h             ; November
                defb    31h             ; December

                defs    6

; Name lookup table for the month, continued.
;
                defm    ' Oct'
                defm    ' Nov'
                defm    ' Dec'

; Name lookup table for the time zone.
;
tznames:        defm    ' UTC'
                defm    ' CET'
                defm    'CEST'

; Day of week delta for 2-digit BCD year.
;
weekdeltayear:  defb    0, 1, 2, 3      ; 00
                defb    5, 6, 0, 1
                defb    3, 4
                defs    6
                defb          5, 6      ; 10
                defb    1, 2, 3, 4
                defb    6, 0, 1, 2
                defs    6
                defb    4, 5, 6, 0      ; 20
                defb    2, 3, 4, 5
                defb    0, 1
                defs    6
                defb          2, 3      ; 30
                defb    5, 6, 0, 1
                defb    3, 4, 5, 6
                defs    6
                defb    1, 2, 3, 4      ; 40
                defb    6, 0, 1, 2
                defb    4, 5
                defs    6
                defb          6, 0      ; 50
                defb    2, 3, 4, 5
                defb    0, 1, 2, 3
                defs    6
                defb    5, 6, 0, 1      ; 60
                defb    3, 4, 5, 6
                defb    1, 2
                defs    6
                defb          3, 4      ; 70
                defb    6, 0, 1, 2
                defb    4, 5, 6, 0
                defs    6
                defb    2, 3, 4, 5      ; 80
                defb    0, 1, 2, 3
                defb    5, 6
                defs    6
                defb          0, 1      ; 90
                defb    3, 4, 5, 6
                defb    1, 2, 3, 4

; Day of week delta for packed BCD month.
;
weekdeltamonth: defb    5               ; January
                defb    1               ; February
                defb    0               ; March
                defb    3               ; April
                defb    5               ; May
                defb    1               ; June
                defb    3               ; July
                defb    6               ; August
                defb    2               ; September
                defs    6
                defb    4               ; October
                defb    0               ; November
                defb    2               ; December

; Day of week delta for packed BCD day of month.
;
weekdeltaday:   defb    0, 1, 2, 3, 4, 5, 6     ; 01 - 07
                defb    0, 1                    ; 08 - 09
                defs    6
                defb          2, 3, 4, 5, 6     ; 10 - 14
                defb    0, 1, 2, 3, 4           ; 15 - 19
                defs    6
                defb                   5, 6     ; 20 - 21
                defb    0, 1, 2, 3, 4, 5, 6     ; 22 - 28
                defb    0                       ; 29
                defs    6
                defb       1, 2                 ; 30 - 31

; Century number corresponding to day of week difference.
; A new century in the sense used here starts on 1 March '00.
;
centfromwdelta: defb    0, 22h, 0, 19h, 20h, 0, 21h     ;  1     -  7 (0)
                defb    0, 22h, 0, 19h, 20h, 0, 21h     ;  8 (1) - 14 (0)
                defb    0, 22h, 0, 19h, 20h, 0, 21h     ; 15 (1) - 21 (0)
                defb    0, 22h, 0, 19h                  ; 22 (1) - 25 (4)

; LCD control register initialization sequence.
;
lcdinitseq:     defb    00111000b       ; 8-bit mode, two rows, 5x7 font
                defb    00000001b       ; clear display
                defb    00000110b       ; entry mode set
                defb    00001100b       ; display on
                defb    01000000b       ; set CG RAM address 0

LCDINITLEN:     equ     $-lcdinitseq

; LCD character graphics data for additional symbols.
;
lcdcgdata:      defb    00000b          ; radio antenna off
                defb    00000b
                defb    00100b
                defb    00100b
                defb    00100b
                defb    01110b
                defb    01110b
                defb    00000b

                defb    10001b          ; radio antenna distortion
                defb    01010b
                defb    10101b
                defb    00100b
                defb    00100b
                defb    01110b
                defb    01110b
                defb    00000b

                defb    01100b          ; radio antenna left wave
                defb    10000b
                defb    10100b
                defb    00100b
                defb    00100b
                defb    01110b
                defb    01110b
                defb    00000b

                defb    00110b          ; radio antenna right wave
                defb    00001b
                defb    00101b
                defb    00100b
                defb    00100b
                defb    01110b
                defb    01110b
                defb    00000b

LCDCGDATALEN:   equ     $-lcdcgdata
