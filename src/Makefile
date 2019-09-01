TOOLPATH = ../tolset/z_tools/
INCPATH  = ../tolset/z_tools/haribote/

MAKE     = make -r
CC1      = cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
RULEFILE = $(TOOLPATH)haribote/haribote.rul
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = cp
DEL      = rm

default :
	$(MAKE) img

%.asm : %.nas Makefile
	\cat $*.nas > $*.asm

%.bin : %.asm Makefile
	nasm $*.asm -o $*.bin -l $*.lst

nasmfunc.o : nasmfunc.asm Makefile
	nasm -g -f elf nasmfunc.asm -o nasmfunc.o

convert_hankaku.out : convert_hankaku.c Makefile
	gcc convert_hankaku.c -o convert_hankaku.out

hankaku.c : convert_hankaku.out hankaku.txt Makefile
	./convert_hankaku.out

bootpack.hrb : bootpack.c hankaku.c graphic.c dsctbl.c int.c har.ld nasmfunc.o Makefile
	gcc -fno-pic -march=i486 -m32 -nostdlib -T har.ld -g bootpack.c hankaku.c \
	graphic.c dsctbl.c int.c fifo.c kbc.c nasmfunc.o -o bootpack.hrb

haribote.sys : asmhead.bin bootpack.hrb Makefile
	\cat asmhead.bin bootpack.hrb > haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i haribote.img ::
	mcopy haribote.sys -i haribote.img ::

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	qemu-system-i386 -fda haribote.img		
	
#"-fda" for floppy disk
	
install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL)	*.o
	-$(DEL) *.asm
	-$(DEL) *.img
	-$(DEL) bootpack.map
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	-$(DEL) convert_hankaku.out
	-$(DEL)	hankaku.c

debug :
	make img
	qemu-system-i386 -fda haribote.img -gdb tcp::10000 -S

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
