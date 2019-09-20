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

%.bin : %.asm Makefile
	nasm $*.asm -o $*.bin -l $*.lst

nasmfunc.o : nasmfunc.asm Makefile
	nasm -g -f elf nasmfunc.asm -o nasmfunc.o

convert_hankaku.o : convert_hankaku.c Makefile
	gcc convert_hankaku.c -o convert_hankaku.o

hankaku.c : convert_hankaku.o hankaku.txt Makefile
	./convert_hankaku.o

bootpack.o : bootpack.c hankaku.c graphic.c dsctbl.c int.c har.ld nasmfunc.o Makefile
	gcc -fno-pic -march=i486 -m32 -nostdlib -T har.ld -g bootpack.c hankaku.c \
	graphic.c dsctbl.c int.c fifo.c kbc.c mouse.c memory.c sheet.c timer.c nasmfunc.o -o bootpack.o

os.sys : asmhead.bin bootpack.o Makefile
	\cat asmhead.bin bootpack.o > os.sys

os.img : ipl10.bin os.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i os.img ::
	mcopy os.sys -i os.img ::

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
	-$(DEL) *.gas
	-$(DEL)	*.o
	-$(DEL) *.img
	-$(DEL) bootpack.map
	-$(DEL) os.sys
	-$(DEL) convert_hankaku.o
	-$(DEL)	hankaku.c

debug :
	make img
	qemu-system-i386 -fda os.img -gdb tcp::10000 -S

src_only :
	$(MAKE) clean
	-$(DEL) os.img
