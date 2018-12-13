all: missile

missile: missile.exe
	../elfconvert missile.exe
	mv missile.exe.converted missile
	cp -f missile ../fsdir/missile

missile.exe: missile-command.o mp1.o mp1-given.o vga.o ece391syscall.o ece391support.o
	gcc -nostdlib -ffreestanding -g -o $@ $^

%.o: %.S
	gcc -nostdlib -ffreestanding -c -Wall -g -D_USERLAND -D_ASM -o $@ $<

%.o: %.c
	gcc -nostdlib -ffreestanding -Wall -c -g -o $@ $<

clean:
	rm -f *.o missile missile.exe
