/*===== main =====*/

#include<stdio.h>
#include"bootpack.h"

void main()
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	char s[100], mcursor[256], keybuf[32], mousebuf[128];

	init_gdtidt();			// GDT IDT initialization
	init_pic();				// PIC initialization
	io_sti(); 				/* IDT/PIC�̏��������I������̂�CPU�̊��荞�݋֎~������ */
	fifo8_init(&keyfifo, 32, keybuf);							// FIFO keyboard initialization
	fifo8_init(&mousefifo, 128, mousebuf);						// FIFO mouse initialization
	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */
	init_keyboard();


	init_palette();												// color palette settings
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);		// screen initialization
	
	/* mouse cursor */
	int mx = (binfo->scrnx - 16) / 2; 
	int	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprint(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	enable_mouse();
	
	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0){
			io_stihlt();
		}else{
			if(fifo8_status(&keyfifo) != 0){
				unsigned char i = fifo8_get(&keyfifo);	// unsigned ???
				char s[40];
				io_sti();
				sprint(s, "%x", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			}else if(fifo8_status(&mousefifo) != 0){
				unsigned char i = fifo8_get(&mousefifo);
				char s[40];
				io_sti();
				sprint(s, "%x", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
			}
		}
	}
}


