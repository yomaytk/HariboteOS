ipl.asm : ipl.nas Makefile
	type ipl.nas > ipl.asm

ipl.bin : ipl.asm Makefile
	nasm -f bin ipl.asm -o ipl.bin -l ipl.lst

tail.bin : tail.asm Makefile
	nasm -f bin tail.asm -o tail.bin -l tail.lst

HariboteOS.img : ipl.bin tail.bin Makefile
	type tail.bin > hariboteOS.img

asm :
	make -r ipl.bin

img :
	make -r tail.bin

