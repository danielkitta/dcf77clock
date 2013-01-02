; vim:ft=z80:sts=8:sw=8:et:
;
; DCF77 clock
; Common definitions
;

; Memory size
;
ROMSTART:       equ     0000h
ROMSIZE:        equ     8192
RAMSTART:       equ     8000h
RAMSIZE:        equ     32768

; Interrupt vector table location
;
INTVH:          equ     ROMSTART/100h+1 ; Interrupt vector table (high byte)
CTCINTVL:       equ     00h             ; CTC interrupt vector (low byte)

; Counter/Timer Circuit I/O port address range
;
CTC0:           equ     10h             ; CTC channel 0
CTC1:           equ     CTC0+1          ; CTC channel 1
CTC2:           equ     CTC0+2          ; CTC channel 2
CTC3:           equ     CTC0+3          ; CTC channel 3

; LCD module I/O port address range
;
LCDCW:		equ	20h             ; LCD control register write
LCDCR:		equ	LCDCW+1         ; LCD control register read
LCDDW:		equ	LCDCW+2         ; LCD data register write
LCDDR:		equ	LCDCW+3         ; LCD data register read

; LCD cursor position commands
;
LCDROW0:        equ     80h             ; first row, leftmost column
LCDROW1:        equ     80h|40h         ; second row, leftmost column
LCDPOSSTAT:     equ     LCDROW0+0       ; status (radio antenna symbol)
LCDPOSTIME:     equ     LCDROW0+2       ; time (hh:mm:ss)
LCDPOSZONE:     equ     LCDROW0+12      ; time zone abbreviation
LCDPOSDATE:     equ     LCDROW1+0       ; date string

; Date/time structure offsets
;
I_VALID:        equ     0               ; date/time is valid (0..1)
I_MIN:          equ     1               ; minute (00..59 BCD)
I_HOUR:         equ     2               ; hour (00..23 BCD)
I_TIMEZONE:     equ     3               ; UTC offset (0..2)
I_DAY:          equ     4               ; day of month (01..31 BCD)
I_WEEKDAY:      equ     5               ; day of week (1..7)
I_MONTH:        equ     6               ; month (01..12 BCD)
I_YEAR:         equ     7               ; year (00..99 BCD)
I_SEC:          equ     8               ; second (00..60 BCD)
I_DISCONT:      equ     9               ; flag discontinuity ahead

DATETIMELEN:    equ     10              ; length of date/time record

; Date/time discontinuity type bits
;
LEAPSEC:        equ     0               ; leap second announcement
DSTSWITCH:      equ     1               ; switch to/from daylight saving time

; Status structure offsets
;
I_STATUS:       equ     10h             ; Status flags
I_CTC2CTRL:     equ     11h             ; CTC channel 2 control word
I_BITCOUNT:     equ     12h             ; DCF77 bit count
I_BITNOW:       equ     13h             ; last received bit, if any
I_EVENTAGE:     equ     14h             ; last event age timeout

; Status flag bits
;
DCFSYNC:        equ     0               ; synchronized on minute start
DCFERROR:       equ     1               ; error during DCF77 reception
DCFCOMMIT:      equ     2               ; received time ready for commit
INCSEC:         equ     3               ; request to increment date/time
DCFEDGE:        equ     4               ; DCF77 pulse edge detected

; DCF77 bit indicators
;
BITPULSE:       equ     0               ; DCF77 pulse active
BITERR:         equ     1               ; receive error
BIT0:           equ     2               ; 0-bit received
BIT1:           equ     3               ; 1-bit received

; Global data
;
DATETIME:       equ     RAMSTART
DT_VALID:       equ     DATETIME+I_VALID
DT_MIN:         equ     DATETIME+I_MIN
DT_HOUR:        equ     DATETIME+I_HOUR
DT_TIMEZONE:    equ     DATETIME+I_TIMEZONE
DT_DAY:         equ     DATETIME+I_DAY
DT_WEEKDAY:     equ     DATETIME+I_WEEKDAY
DT_MONTH:       equ     DATETIME+I_MONTH
DT_YEAR:        equ     DATETIME+I_YEAR
DT_SEC:         equ     DATETIME+I_SEC
DT_DISCONT:     equ     DATETIME+I_DISCONT

STATUS:         equ     RAMSTART+I_STATUS
CTC2CTRL:       equ     RAMSTART+I_CTC2CTRL
BITCOUNT:       equ     RAMSTART+I_BITCOUNT
BITNOW:         equ     RAMSTART+I_BITNOW
EVENTAGE:       equ     RAMSTART+I_EVENTAGE

WRAPTIMER:      equ     RAMSTART+20h    ; wrap-around time counter
PULSESTART:     equ     RAMSTART+22h    ; DCF77 pulse start time
PULSESTOP:      equ     RAMSTART+24h    ; DCF77 pulse stop time
RECTIMEPTR:     equ     RAMSTART+26h    ; pointer to temporary date/time record

RECTIME1:       equ     RAMSTART+40h    ; received temporary time stamp 1
RECTIME2:       equ     RAMSTART+50h    ; received temporary time stamp 2

DCFBITS:        equ     RAMSTART+80h    ; received DCF77 bits
