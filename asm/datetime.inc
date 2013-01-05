; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Date/time handling
;

; Increment date/time record by one second.
; Input  : iy
; Output : -
; Scratch: af, bc, de, hl
;
incdatetimesec: ld      de, 0059h
                bit     LEAPSEC, (iy+I_DISCONT) ; leap second announcement?
                jp      z, .skipleapsec

                ld      a, (iy+I_MIN)
                cp      e                       ; 59th minute?
                jr      nz, .skipleapsec        ; no: skip

                ld      e, 60h                  ; otherwise insert leap second

.skipleapsec:   ld      b, 1                    ; increment by 1
                ld      a, (iy+I_SEC)
                call    .incbcd                 ; increment second
                ld      (iy+I_SEC), a
                ret     nc

; Increment date/time record by one minute.
; Input  : iy
; Output : -
; Scratch: af, bc, de, hl
;
incdatetimemin: ld      de, 0059h
                ld      b, 1                    ; increment by 1
                ld      a, (iy+I_MIN)
                call    .incbcd                 ; increment minute
                ld      (iy+I_MIN), a
                ret     nc

                ld      e, 23h
                bit     DSTSWITCH, (iy+I_DISCONT)
                ld      (iy+I_DISCONT), d       ; clear discontinuity flags
                jp      z, .skipdst

                ld      a, (iy+I_TIMEZONE)
                xor     11b                     ; flip CET/CEST (UTC+1/2)
                jp      pe, .skipdst            ; skip if some other timezone
                ld      (iy+I_TIMEZONE), a
                dec     a                       ; switch from CEST to CET?
                ret     z                       ; yes: skip hour increment

                inc     b                       ; otherwise increment hour by 2
                dec     e                       ; wrap around after 22h

.skipdst:       ld      a, (iy+I_HOUR)
                call    .incbcd                 ; increment hour
                ld      (iy+I_HOUR), a
                ret     nc

                ld      de, 0107h
                ld      b, d                    ; increment by 1
                ld      a, (iy+I_WEEKDAY)
                call    .incbcd                 ; increment day of week
                ld      (iy+I_WEEKDAY), a

                ld      hl, dayspermonth-101h   ; correct for d=1
                ld      e, (iy+I_MONTH)
                add     hl, de                  ; look up
                ld      e, (hl)                 ; number of days per month
                ld      a, 28h
                cp      e                       ; February?
                jr      nz, .skipleapday        ; no: skip

                cp      (iy+I_DAY)              ; 28th?
                jr      nz, .skipleapday        ; no: skip

                ld      a, (iy+I_YEAR)
                ld      c, a                    ; remember year
                bit     4, a                    ; first digit is odd?
                jr      z, .skipbcdfix          ; no: skip BCD correction

                add     a, 2                    ; correction for divisibility

.skipbcdfix:    and     3                       ; multiple of 4?
                jr      nz, .skipleapday        ; no: cannot be a leap year

                or      c                       ; year 00?
                jr      nz, .leapday            ; no: always a leap year

                ld      a, (iy+I_WEEKDAY)
                xor     2                       ; next day of week is Tuesday?
                jr      nz, .skipleapday        ; no: not a leap year

.leapday:       inc     e                       ; insert leap day

.skipleapday:   ld      a, (iy+I_DAY)
                call    .incbcd                 ; increment day of month
                ld      (iy+I_DAY), a
                ret     nc

                ld      e, 12h
                ld      a, (iy+I_MONTH)
                call    .incbcd                 ; increment month
                ld      (iy+I_MONTH), a
                ret     nc

                ld      de, 0099h
                ld      a, (iy+I_YEAR)
                call    .incbcd                 ; increment year
                ld      (iy+I_YEAR), a
                ret

; Increment value stored as two packed BCD digits.
; Input  : a, b, de
; Output : a, cy
; Scratch: f
;
.incbcd:        cp      e                       ; compare against upper bound
                jr      nc, .incwrap            ; above or equal: wrap around

                add     a, b                    ; increment packed BCD
                daa
                ret

.incwrap:       ld      a, d                    ; wrap around to lower bound
                scf
                ret

; Calculate the century from date with 2-digit year and given day of week.
; Input  : iy
; Output : a, cy
; Scratch: af, bc, de, hl
;
calccentury:    ld      a, (iy+I_MONTH)
                ld      b, 0
                ld      c, a                    ; remember month
                cp      03h                     ; month is March or later?
                ld      a, (iy+I_YEAR)
                sbc     a, b                    ; no: decrement year
                daa
                ld      d, b                    ; d=0
                ld      e, a
                ld      hl, weekdeltayear
                add     hl, de
                ld      a, (hl)                 ; get year's day of week delta
                ld      hl, weekdeltamonth-1
                add     hl, bc
                add     a, (hl)                 ; add month's day of week delta
                ld      c, (iy+I_DAY)
                ld      hl, weekdeltaday-1
                add     hl, bc
                add     a, (hl)                 ; add day's day of week delta
                add     a, 7                    ; add bias to keep delta >= 0
                sub     (iy+I_WEEKDAY)          ; calc day of week difference
                ld      c, a
                ld      hl, centfromwdelta
                add     hl, bc
                ld      a, (iy+I_MONTH)
                cp      03h                     ; month is March or later?
                ld      a, e
                adc     a, b                    ; no: increment year
                daa
                ld      a, (hl)                 ; look up century
                adc     a, b                    ; apply correction if needed
                daa
                cp      19h                     ; cy=1 if century invalid
                ret
