# TOOLPATH = ../z_tools/
# MAKE     = $(TOOLPATH)make.exe -r
# NASK     = $(TOOLPATH)nask.exe
# EDIMG    = $(TOOLPATH)edimg.exe
# IMGTOL   = $(TOOLPATH)imgtol.com
# COPY     = copy
# DEL      = del

# # デフォルト動作

# default :
# 	$(MAKE) img

# # ファイル生成規則

# ipl.bin : ipl.nas Makefile
# 	$(NASK) ipl.nas ipl.bin ipl.lst

# haribote.sys : haribote.nas Makefile
# 	$(NASK) haribote.nas haribote.sys haribote.lst

# haribote.img : ipl.bin haribote.sys Makefile
# 	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
# 		wbinimg src:ipl.bin len:512 from:0 to:0 \
# 		copy from:haribote.sys to:@: \
# 		imgout:haribote.img

# # コマンド

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
# 	-$(DEL) haribote.sys
# 	-$(DEL) haribote.lst

# src_only :
# 	$(MAKE) clean
# 	-$(DEL) haribote.img

ipl.asm : ipl.nas Makefile
	type ipl.nas > ipl.asm

ipl.bin : ipl.asm Makefile
	nasm -f bin ipl.asm -o ipl.bin -l ipl.lst

tail.bin : tail.asm Makefile
	nasm -f bin tail.asm -o tail.bin -l tail.lst

os.img : ipl.bin Makefile
	type ipl.bin > hariboteOS03e.img

asm :
	make -r ipl.bin

img :
	make -r tail.bin

# haribote.img : ipl.bin haribote.sys Makefile
#     mformat -f 1440 -C -B ipl.bin -i haribote.img ::
    # mcopy haribote.sys -i haribote.img ::
