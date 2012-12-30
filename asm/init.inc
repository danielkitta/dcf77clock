; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Initialization on reset
;

.NOP:           equ     00h                     ; opcode of nop
.PUSH_HL:       equ     0E5h                    ; opcode of push hl

                org     ROMSTART
;
; Entry point on reset
;
start:          jp      .init

                defs    ROMSTART+80h-$, .NOP    ; reserved (RST entry points)

.init:          di
                im      2                       ; Z80 vectored interrupt mode
                ld      a, INTVH
                ld      i, a                    ; high byte of int vector table

                ld      hl, 0
                ld      b, h
                ld      sp, 0FFFFh&(RAMSTART+RAMSIZE)

.ramclear:      defs    RAMSIZE/200h, .PUSH_HL  ; fast RAM clear
                djnz    .ramclear               ; (45 ms at 4.096 MHz)

                ld      sp, RAMSTART+RAMSIZE-2  ; set up stack

                ld      a, CTCINTVL
                out     (CTC0), a               ; program CTC interrupt vector

                ld      ix, RAMSTART            ; globally constant
                ld      hl, RECTIME1            ; current temp time destination
                ei
                ld      (RECTIMEPTR), hl

                ld      (ix+I_DAY), 1           ; initialize to a valid date
                ld      (ix+I_WEEKDAY), 6       ; 2000-01-01 was a Saturday
                ld      (ix+I_MONTH), 1

                call    resetlcd
                call    resetsectimer
                call    resetdcftimer

                jp      main

                defs    INTVH*100h+CTCINTVL-$   ; fill up space to int table

; CTC interrupt vector table
;
                defw    intret                  ; CTC channel 0 (4 ms timer)
                defw    ctc1int                 ; CTC channel 1 (seconds timer)
                defw    ctc2int                 ; CTC channel 2 (DCF77 signal)
                defw    ctc3int                 ; CTC channel 3 (wait timeout)

                defs    INTVH*100h+100h-$       ; fill to 256-byte boundary
