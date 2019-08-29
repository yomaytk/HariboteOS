TOOLPATH = ../tolset/z_tools/
INCPATH  = ../tolset/z_tools/haribote/

MAKE     = make -r
NASK     = $(TOOLPATH)nask.exe
CC1      = cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
MAKEFONT = $(TOOLPATH)makefont.exe
COPY     = cp
DEL      = rm

default :
	$(MAKE) img

ipl10.asm : ipl10.nas Makefile
	\cat ipl10.nas > ipl10.asm

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.asm : asmhead.nas Makefile
	\cat asmhead.nas > asmhead.asm

asmhead.bin : asmhead.asm Makefile
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

nasmfunc.asm : naskfunc.nas Makefile
	\cat naskfunc.nas > nasmfunc.asm

nasmfunc.o : nasmfunc.asm Makefile
	nasm -g -f elf nasmfunc.asm -o nasmfunc.o

convert_hankaku.out : convert_hankaku.c Makefile
	gcc convert_hankaku.c -o convert_hankaku.out

hankaku.c : convert_hankaku.out hankaku.txt Makefile
	./convert_hankaku.out

bootpack.hrb : bootpack.c hankaku.c har.ld nasmfunc.o Makefile
	gcc -fno-pic -march=i486 -m32 -nostdlib -T har.ld -g bootpack.c hankaku.c nasmfunc.o -o bootpack.hrb

haribote.sys : asmhead.bin bootpack.hrb Makefile
	\cat asmhead.bin bootpack.hrb > haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i haribote.img ::
	mcopy haribote.sys -i haribote.img ::

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	qemu-system-i386 -fda haribote.img		#"-fda" for floppy disk
	
install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL)	*.o
	-$(DEL) *.asm
	-$(DEL) *.img
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
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
