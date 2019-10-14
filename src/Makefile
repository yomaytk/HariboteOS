TOOLPATH = ../tolset/z_tools/
INCPATH  = ../tolset/z_tools/os/

MAKE     = make -r
CC1      = cc1.exe -I$(INCPATH) -Os -Wall -quiet
RULEFILE = $(TOOLPATH)os/os.rul
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = cp
DEL      = rm

default :
	$(MAKE) img

%.asm : %.nas Makefile
	\cat $*.nas > $*.asm

%.s : %.asm Makefile
	\cat $*.asm > $*.s

%.bin : %.s Makefile
	nasm $*.s -o $*.bin -l $*.lst

nasmfunc.o : nasmfunc.s Makefile
	nasm -g -f elf nasmfunc.s -o nasmfunc.o

convert_hankaku.o : convert_hankaku.c Makefile
	gcc convert_hankaku.c -o convert_hankaku.o

hankaku.c : convert_hankaku.o hankaku.txt Makefile
	./convert_hankaku.o

bootpack.o : bootpack.c hankaku.c graphic.c dsctbl.c int.c os.ld nasmfunc.o Makefile
	gcc -Wl,-Map=bootpack.map -fno-pic -march=i486 -m32 -nostdlib -T os.ld -g bootpack.c hankaku.c \
	graphic.c dsctbl.c int.c fifo.c kbc.c mouse.c memory.c sheet.c timer.c \
	taskfunc.c mtask.c cmd.c mss_libc32.c file.c exception.c nasmfunc.o -o bootpack.o

a.o : a_nasm.s Makefile
	nasm -g -f elf a_nasm.s -o a.o

app.o : a.c a.o Makefile
	gcc -Wl,-Map=app.map -fno-pic -march=i486 -m32 -nostdlib -T os.ld -g a.c a.o -o app.o

crack1.o : crack1.c a.o Makefile
	gcc -fno-pic -march=i486 -m32 -nostdlib -T os.ld -g crack1.c a.o -o crack1.o

hello3.o : hello3.c a.o Makefile
	gcc -Wl,-Map=hello3.map -fno-pic -march=i486 -m32 -nostdlib -T os.ld -g hello3.c a.o -o hello3.o

bug1.o :
	gcc -fno-pic -march=i486 -m32 -nostdlib -T os.ld -g bug1.c a.o -o bug1.o

win_hel.o :
	gcc -fno-pic -march=i486 -m32 -nostdlib -T os.ld -g win_hello.c a.o -o win_hel.o

os.sys : asmhead.bin bootpack.o Makefile
	\cat asmhead.bin bootpack.o > os.sys

os.img : ipl10.bin os.sys hello2.bin app.o hello3.o bug2.bin bug3.bin win_hel.o Makefile
	mformat -f 1440 -C -B ipl10.bin -i os.img ::
	mcopy os.sys -i os.img ::
	mcopy hello2.bin -i os.img ::
	mcopy app.o -i os.img ::
	mcopy hello3.o -i os.img ::
	mcopy bug2.bin -i os.img ::
	mcopy bug3.bin -i os.img ::
	mcopy win_hel.o -i os.img ::

img :
	$(MAKE) os.img

run :
	$(MAKE) img
	qemu-system-i386 -fda os.img	
	
#"-fda" for floppy disk
	
install :
	$(MAKE) img
	$(IMGTOL) w a: os.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL)	*.o
	-$(DEL) *.img
	-$(DEL) *.map
	-$(DEL) os.sys
	-$(DEL)	hankaku.c

debug :
	make img
	qemu-system-i386 -fda os.img -gdb tcp::10000 -S

src_only :
	$(MAKE) clean
	-$(DEL) os.img

# test
test.o : test.c Makefile
	gcc test.c -o test.o

te: test.o Makefile
	./test.o
	rm test.o
