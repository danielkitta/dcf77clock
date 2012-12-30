# build rules for dcfclock

AS = z80asm
DEPS = cdata.inc common.inc datetime.inc decode.inc init.inc lcd.inc main.inc timer.inc

all: dcfclock.bin

clean:
	rm -f dcfclock.bin

dcfclock.bin: dcfclock.asm $(DEPS)
	$(AS) -o $@ dcfclock.asm
