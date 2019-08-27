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
COPY     = cp
DEL      = rm

default :
	$(MAKE) img

ipl10.asm : ipl10.nas Makefile
	\cat ipl10.nas > ipl10.asm

ipl10.bin : ipl10.asm Makefile
	nasm -f bin ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.asm : asmhead.nas Makefile
	\cat asmhead.nas > asmhead.asm

asmhead.bin : asmhead.asm Makefile
	nasm -f bin asmhead.asm -o asmhead.bin -l asmhead.lst

bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas Makefile
	gas2nask.exe -a bootpack.gas bootpack.nas

bootpack.asm : bootpack.nas Makefile
	\cat bootpack.nas > bootpack.asm

bootpack.obj : bootpack.asm Makefile
	nasm -fwin32 bootpack.asm -l bootpack.lst

nasmfunc.asm : naskfunc.nas Makefile
	\cat naskfunc.nas > nasmfunc.asm

naskfunc.o : nasmfunc.asm Makefile
	nasm -g -f elf nasmfunc.asm -o nasmfunc.o

bootpack.bim : bootpack.obj naskfunc.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj
# 3MB+64KB=3136KB

# bootpack.hrb : bootpack.bim Makefile
# 	$(BIM2HRB) bootpack.bim bootpack.hrb 0

bootpack.hrb : bootpack.c har.ld nasmfunc.o Makefile
	gcc -fno-pic -march=i486 -m32 -nostdlib -T har.ld -g bootpack.c nasmfunc.o -o bootpack.hrb

haribote.sys : asmhead.bin bootpack.hrb Makefile
	\cat asmhead.bin bootpack.hrb > haribote.sys

# haribote.img : ipl10.bin haribote.sys Makefile
# 	$(EDIMG)   imgin:../tolset/z_tools/fdimg0at.tek \
# 		wbinimg src:ipl10.bin len:512 from:0 to:0 \
# 		copy from:haribote.sys to:@: \
# 		imgout:haribote.img
haribote.img : ipl10.bin haribote.sys Makefile
    # mformat -f 1440 -C -B ipl10.bin -i haribote.img ::
    # mcopy haribote.sys -i haribote.img ::

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	qemu-system-i386 -fda haribote.img  # "-fda" for floppy disk
	# $(COPY) haribote.img ..\tolset\z_tools\qemu\fdimage0.bin
	# $(MAKE) -C ../tolset/z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.asm
	-$(DEL) *.img
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
