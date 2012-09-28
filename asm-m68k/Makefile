# This file is Public Domain

AS = @m68k-amigaos-as -m68000
LD = @m68k-amigaos-ld
STRIP = @m68k-amigaos-strip

EXES = amiga_hello_world bootstrap

GENHDR = bootstrap.h

all: $(EXES) $(GENHDR)

amiga_hello_world: amiga_hello_world.o

bootstrap: bootstrap.o

%: %.o
	$(LD) -o $@ $<
	$(STRIP) $@

bootstrap.raw:	bootstrap
	$(STRIP) -o bootstrap.raw -O binary bootstrap

bootstrap.h:	bootstrap.raw
	echo "/* Emumiga bootstrap. Generated from /asm-m68k/bootstrap.s */" >bootstrap.h
	echo "uint8_t const emulator::bootstrap_data[] = {" >>bootstrap.h
	hexdump -v -e '"\t" 4/1 "0x%02X," "\n"' bootstrap.raw >>bootstrap.h
	echo "};" >>bootstrap.h

clean:
	rm -f *.o *.raw $(EXES) $(GENHDR)
