# TOOLPATH = ../z_tools/
# MAKE     = $(TOOLPATH)make.exe -r
# NASK     = $(TOOLPATH)nask.exe
# EDIMG    = $(TOOLPATH)edimg.exe
# IMGTOL   = $(TOOLPATH)imgtol.com
# COPY     = copy
# DEL      = del

# # �f�t�H���g����

# default :
# 	$(MAKE) img

# # �t�@�C�������K��

# ipl.bin : ipl.nas Makefile
# 	$(NASK) ipl.nas ipl.bin ipl.lst

# haribote.img : ipl.bin Makefile
# 	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
# 		wbinimg src:ipl.bin len:512 from:0 to:0   imgout:haribote.img

# # �R�}���h

# asm :
# 	$(MAKE) ipl.bin

# img :
# 	$(MAKE) haribote.img

# run :
# 	$(MAKE) img
# 	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
# 	$(MAKE) -C ../z_tools/qemu

# install :
# 	$(MAKE) img
# 	$(IMGTOL) w a: haribote.img

# clean :
# 	-$(DEL) ipl.bin
# 	-$(DEL) ipl.lst

# src_only :
# 	$(MAKE) clean
# 	-$(DEL) haribote.img
ipl.asm : ipl.nas Makefile
	type ipl.nas > ipl.asm

ipl.bin : ipl.asm Makefile
	nasm -f bin ipl.asm -o ipl.bin -l ipl.lst

tail.bin : tail.asm Makefile
	nasm -f bin tail.asm -o tail.bin -l tail.lst

img : ipl.bin ipl.bin Makefile
	type ipl.bin > hariboteOS03e.img

asm :
	make -r ipl.bin

img :
	make -r tail.bin
