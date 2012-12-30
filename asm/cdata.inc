; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Constant data segment
;

                defs	-$&0FFh, 0	; align to 256-byte boundary

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
