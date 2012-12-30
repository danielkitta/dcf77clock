; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Timer interrupt routines
;

; CTC control bits
;
.CTC_INT:       equ     1<<7
.CTC_TIMER:     equ     0
.CTC_COUNT:     equ     1<<6
.CTC_PRE16:     equ     0
.CTC_PRE256:    equ     1<<5
.CTC_FALLING:   equ     0
.CTC_RISING:    equ     1<<4
.CTC_AUTO:      equ     0
.CTC_TRIG:      equ     1<<3
.CTC_TCFOLLOW:  equ     1<<2
.CTC_CONT:      equ     0
.CTC_RESET:     equ     1<<1
.CTC_CTRL:      equ     1<<0

; CTC channel 0 setup:
; Divide 4.096 MHz CLK by 2^14 => output period 4 ms
;
; - disable interrupt
; - timer mode
; - prescaler 256
; - automatic trigger
; - software reset
;
.CTC0_CW:       equ     .CTC_TIMER|.CTC_PRE256|.CTC_AUTO|.CTC_RESET|.CTC_CTRL
.CTC0_TC:       equ     64

; CTC channel 1 setup:
; Divide channel 0 output by 250 => interrupt period 1 s
;
; - enable interrupt
; - counter mode
; - clock on rising edge
; - software reset
;
.CTC1_CW:       equ     .CTC_INT|.CTC_COUNT|.CTC_RISING|.CTC_RESET|.CTC_CTRL
.CTC1_TC:       equ     250

; CTC channel 2 setup:
; Divide 4.096 MHz CLK by 2^16 => output period 16 ms
;
; - enable interrupt
; - timer mode
; - prescaler 256
; - trigger on falling edge
; - software reset
;
.CTC2_CW:       equ     .CTC_INT|.CTC_TIMER|.CTC_PRE256|.CTC_FALLING|.CTC_TRIG|.CTC_RESET|.CTC_CTRL
.CTC2_TC:       equ     0

; CTC channel 3 setup (off):
;
; - timer mode
; - automatic trigger
; - software reset
;
.CTC3_CW:       equ     .CTC_TIMER|.CTC_AUTO|.CTC_RESET|.CTC_CTRL

; DCF77 pulse detection thresholds.
;
.PULSE0_MIN:    equ     .CTC1_TC*7/100          ; min length of 0-bit pulse
.PULSE0_MAX:    equ     .CTC1_TC*13/100         ; max length of 0-bit pulse
.PULSE1_MIN:    equ     .CTC1_TC*17/100         ; min length of 1-bit pulse
.PULSE1_MAX:    equ     .CTC1_TC*24/100         ; max length of 1-bit pulse
.PULSEGAP_MIN:  equ     .CTC1_TC*67/100         ; min pause between pulses
.MINSYNC_MIN:   equ     .CTC1_TC+.PULSEGAP_MIN  ; min pause before new minute

; Reset seconds timer.
; Input  : -
; Output : -
; Scratch: af, hl
;
resetsectimer:  ld      a, .CTC0_CW|.CTC_TCFOLLOW
                out     (CTC0), a               ; CTC channel 0: timer,
                ld      a, .CTC0_TC             ; cascaded with channel 1
                out     (CTC0), a

                ld      a, .CTC1_CW|.CTC_TCFOLLOW
                out     (CTC1), a               ; CTC channel 1: counter,
                ld      a, .CTC1_TC             ; interrupt every second
                out     (CTC1), a

                ld      hl, 0
                ld      (WRAPTIMER), hl         ; reset wrap-around counter
                ld      (PULSESTART), hl        ; reset pulse start time stamp
                ld      h, -8
                ld      (PULSESTOP), hl         ; preset pulse stop time stamp

                res     INCSEC, (ix+I_STATUS)   ; discard pending increment
                ret

; Reset DCF77 pulse timer.
; Input  : -
; Output : -
; Scratch: af
;
resetdcftimer:  ld      a, .CTC2_CW|.CTC_TCFOLLOW
                ld      (CTC2CTRL), a           ; remember current control word
                out     (CTC2), a
                ld      a, .CTC2_TC             ; CTC channel 2: timer,
                out     (CTC2), a               ; debounce DCF77 input

                ld      (ix+I_EVENTAGE), 0FFh   ; reset event age down counter
                xor     a
                ld      (STATUS), a             ; reset all status flags
                ret

; CTC channel 1 interrupt handler.
; Scratch: af', de', hl'
;
ctc1int:        ex      af, af'
                exx

                srl     (ix+I_EVENTAGE)         ; saturating countdown
                jr      z, .freezetimer         ; max age: limit timer delta

                ld      de, .CTC1_TC
                ld      hl, (WRAPTIMER)         ; advance wrap-around timer
                add     hl, de                  ; by channel 1 period
                ld      (WRAPTIMER), hl

.freezetimer:   set     INCSEC, (ix+I_STATUS)   ; issue seconds increment

                exx
                ex      af, af'
intret:         ei
                reti

.intresolve:    ei                              ; wait for pending interrupt
                nop                             ; to resolve race condition
                ld      hl, (WRAPTIMER)         ; get timer after possible int
                jp      .ctc2intcont

; CTC channel 2 interrupt handler.
;
ctc2int:        push    af                      ; not using shadow registers:
                push    bc                      ; going to reenable interrupts
                push    de                      ; early on for possible timer
                push    hl                      ; adjustment
                push    iy

                ld      (ix+I_EVENTAGE), 0FFh   ; reset event age down counter

                in      a, (CTC1)               ; sample time counter
                dec     a
                cp      .CTC1_TC-1              ; counter value ambiguous?
                jr      nc, .intresolve         ; yes: resolve ambiguity

                ei                              ; otherwise reenable ints...
                ld      hl, (WRAPTIMER)         ; ...after timer fetch
                cpl
                add     a, .CTC1_TC             ; reverse counter value
                ld      e, a
                ld      d, 0
                add     hl, de                  ; add delta to overall timer

.ctc2intcont:   ld      a, (CTC2CTRL)
                xor     .CTC_RISING             ; toggle active edge
                ld      (CTC2CTRL), a
                out     (CTC2), a               ; reset timer with new edge
                and     .CTC_RISING             ; was falling edge?
                ld      a, .CTC2_TC
                out     (CTC2), a
                set     DCFEDGE, (ix+I_STATUS)
                jp      z, .pulsestop           ; saw rising edge -> stop

                ld      (PULSESTART), hl        ; record time of falling edge
                ld      de, (PULSESTOP)
                xor     a                       ; a=BITPULSE, cy=0
                ld      (BITNOW), a
                sbc     hl, de
                or      h                       ; delta >= 256?
                jr      nz, .minutemark         ; yes: minute marker

                ld      a, .PULSEGAP_MIN-1
                cp      l                       ; minimum pause time met?
                jp      c, .ctc2intret          ; yes: return

.biterror:      ld      (ix+I_BITNOW), BITERR   ; indicate reception error
.oferror:       set     DCFERROR, (ix+I_STATUS) ; remember error status

.ctc2intret:    pop     iy
                pop     hl
                pop     de
                pop     bc
                pop     af
                reti

.pulsestop:     ld      (PULSESTOP), hl         ; record time of rising edge
                ld      de, (PULSESTART)
                xor     a                       ; a=0, cy=0
                sbc     hl, de                  ; calculate pulse width

                or      h                       ; delta < 256?
                jr      nz, .biterror           ; no: error

                ld      a, l
                sub     .PULSE0_MIN
                cp      .PULSE1_MAX+1-.PULSE0_MIN       ; min0 < delta < max1?
                jr      nc, .biterror                   ; no: error

                add     a, .PULSE0_MIN-.PULSE0_MAX-1
                cp      .PULSE1_MIN-.PULSE0_MAX-1       ; max0 < delta < min1?
                jr      c, .biterror                    ; yes: error

                rlca                            ; cy=sign
                sbc     a, a                    ; a=-cy
                inc     a
                ld      d, 0
                ld      e, (ix+I_BITCOUNT)      ; index into DCF77 bit array
                ld      hl, DCFBITS
                add     hl, de
                ld      (hl), a                 ; store received bit
                add     a, BIT0
                ld      (BITNOW), a
                inc     e                       ; increment bit count
                jp      m, .oferror             ; overflow: error

                ld      (ix+I_BITCOUNT), e
                jp      .ctc2intret

.minutemark:    dec     a                       ; 256 <= delta < 512?
                jr      nz, .minuteclear        ; no: stale record, no commit

                ld      de, -.MINSYNC_MIN
                add     hl, de                  ; minimum sync pause time met?
                jr      nc, .biterror           ; no: error

                ld      iy, (RECTIMEPTR)        ; fetch current temp record
                bit     0, (iy+I_VALID)         ; already got a valid reading?
                jr      z, .skipincrec          ; no: skip minute increment

                call    incdatetimemin          ; increment prev time by 1 min

                ld      a, (RECTIMEPTR)
                xor     RECTIME1^RECTIME2       ; toggle between locations
                ld      (RECTIMEPTR), a
                ld      iy, (RECTIMEPTR)        ; update pointer

.skipincrec:    call    dcfdecode
                jr      c, .minuteclear         ; decoding error: no commit

                ld      a, (DT_VALID)
                and     a                       ; already displaying date/time?
                jr      z, .skipcompare         ; no: skip double check

                ld      de, RECTIME1
                ld      hl, RECTIME2
                ld      b, DATETIMELEN-2        ; skip seconds and flag bits
.prevcmploop:   ld      a, (de)
                cp      (hl)                    ; compare date/time records
                jr      nz, .minuteclear        ; difference found: no commit
                inc     e
                inc     l
                djnz    .prevcmploop

.skipcompare:   call    resetsectimer           ; sync seconds timer
                set     DCFCOMMIT, (ix+I_STATUS)

.minuteclear:   xor     a
                ld      (BITCOUNT), a
                res     DCFERROR, (ix+I_STATUS) ; reset reception error flag
                set     DCFSYNC, (ix+I_STATUS)  ; start new minute recording
                jp      .ctc2intret

; CTC channel 3 interrupt handler.
; Scratch: af'
;
ctc3int:        ex      af, af'
                ld      a, .CTC3_CW
                out     (CTC3), a               ; turn off timer
                ex      af, af'
                ei
                reti

; Halt until timeout interrupt after ~100 microseconds.
; Input  : -
; Output : -
; Scratch: af
;
halt100us:      ld      a, .CTC3_CW|.CTC_INT|.CTC_PRE16|.CTC_TCFOLLOW
                out     (CTC3), a
                ld      a, 25                   ; divide by 25*2^4
                out     (CTC3), a
                halt                            ; wait for interrupt
                ret

; Halt until timeout interrupt after ~4.1 milliseconds.
; Input  : -
; Output : -
; Scratch: af
;
halt4100us:     ld      a, .CTC3_CW|.CTC_INT|.CTC_PRE256|.CTC_TCFOLLOW
                out     (CTC3), a
                ld      a, 66                   ; divide by 66*2^8
                out     (CTC3), a
                halt                            ; wait for interrupt
                ret
