; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Main program loop
;

; Bit masks corresponding to status flags.
;
.DCFCOMMITMASK: equ     1<<DCFCOMMIT
.DCFEDGEMASK:   equ     1<<DCFEDGE
.INCSECMASK:    equ     1<<INCSEC
.EVENTMASK:     equ     .DCFCOMMITMASK|.DCFEDGEMASK|.INCSECMASK

; Main loop.
;
main:           call    .updatedisplay

.mainloop:      di
                ld      a, (STATUS)             ; poll current status flags
                bit     DCFCOMMIT, a
                jr      nz, .dcfcommit          ; handle DCF77 time commit
                bit     DCFEDGE, a
                jr      nz, .dcfedge            ; handle signal activity
                bit     INCSEC, a
                jr      nz, .incsec             ; handle clock increment

                ei
                halt                            ; wait for next interrupt
                jp      .mainloop

.dcfcommit:     and     ~.EVENTMASK             ; forgo pending increment and
                ld      (STATUS), a             ; reception indicator display

                ld      hl, (RECTIMEPTR)
                ld      de, DATETIME
                ld      bc, DATETIMELEN
                ldir                            ; copy decoded time stamp
                ei

                ld      b, ' '                  ; flash indicator on commit
                call    .showbitstatus
                jp      main                    ; return and update display

.dcfedge:       and     ~.DCFEDGEMASK
                ld      (STATUS), a
                ei
                ld      a, (BITNOW)

                ld      b, a
                call    .showbitstatus          ; display DCF77 bit indicator
                jp      .mainloop               ; return without display update

.incsec:        and     ~.INCSECMASK
                ei
                ld      (STATUS), a

                ld      iy, DATETIME
                call    incdatetimesec
                jp      main                    ; return and update display

; Display DCF77 reception status indicator.
; Input  : b
; Output : -
; Scratch: af, c
;
.showbitstatus: ld      c, LCDPOSSTAT           ; set cursor
                call    writelcdctrl
                ld      c, b
                jp      writelcddata

; Update date/time display.
; Input  : -
; Output : -
; Scratch: af, bc, de, hl
;
.updatedisplay: ld      c, LCDPOSTIME           ; set cursor
                call    writelcdctrl

                ld      a, (DT_HOUR)            ; time in hh:mm:ss format
                call    writelcdbcdslz
                ld      c, ':'
                call    writelcddata
                ld      a, (DT_MIN)
                call    writelcdbcd
                ld      c, ':'
                call    writelcddata
                ld      a, (DT_SEC)
                call    writelcdbcd

                ld      a, (DT_VALID)
                and     a
                ret     z                       ; don't display invalid date

                ld      c, LCDPOSZONE           ; set cursor
                call    writelcdctrl

                ld      a, (DT_TIMEZONE)
                ld      hl, tznames
                call    writelcdtable4          ; name of time zone

                ld      c, LCDPOSDATE           ; set cursor
                call    writelcdctrl

                ld      a, (DT_WEEKDAY)
                ld      hl, daynames-4
                call    writelcdtable4          ; day of week abbreviation

                ld      c, ' '
                call    writelcddata
                ld      a, (DT_DAY)
                call    writelcdbcdslz          ; day of month number

                ld      a, (DT_MONTH)
                ld      hl, monthnames-4
                call    writelcdtable4          ; month name abbreviation

                ld      c, ' '
                call    writelcddata
                ld      iy, DATETIME
                call    calccentury
                jr      c, .nocentury

                call    writelcdbcd             ; 2-digit century
                ld      a, (DT_YEAR)
                jp      writelcdbcd             ; 2-digit year

.nocentury:     ld      c, ' '                  ; blank century
                call    writelcddata
                call    writelcddata
                ld      a, (DT_YEAR)
                jp      writelcdbcd             ; 2-digit year
