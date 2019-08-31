/*===== main =====*/

#include<stdio.h>
#include"bootpack.h"

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	char s[100], mcursor[256];

	init_gdtidt();			// GDT IDT initialization
	init_pic();				// PIC initialization
	io_sti(); 				/* IDT/PIC�̏��������I������̂�CPU�̊��荞�݋֎~������ */

	init_palette();												// color palette settings
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);		// screen initialization

	/* mouse cursor */
	int mx = (binfo->scrnx - 16) / 2; 
	int	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprint(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	for (;;) {
		if(keybuf.flag == 0){
			io_stihlt();
		}else{
			unsigned char i = keybuf.data;
			char s[4];
			keybuf.flag = 0;
			io_sti();
			sprint(s, "%x", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}
}


