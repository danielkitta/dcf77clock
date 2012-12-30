; vim:ft=z80:sts=8:sw=8:et:

include "common.inc"
include "init.inc"
include "main.inc"
include "lcd.inc"
include "timer.inc"
include "decode.inc"
include "datetime.inc"
include "cdata.inc"

                defs    ROMSIZE-$, 0FFh         ; fill up ROM image

                end
