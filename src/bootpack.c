/*===== main =====*/

#include<stdio.h>
#include"bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;

void main()
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	char s[100], mcursor[256], keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, *buf_mouse;


	init_gdtidt();			// GDT IDT initialization
	init_pic();				// PIC initialization
	io_sti(); 				/* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo8_init(&keyfifo, 32, keybuf);				// FIFO keyboard initialization
	fifo8_init(&mousefifo, 128, mousebuf);			// FIFO mouse initialization
	io_out8(PIC0_IMR, 0xf9); 			// PIC1とキーボードを許可(11111001)
	io_out8(PIC1_IMR, 0xef); 			// マウスを許可(11101111)
	init_keyboard();
	enable_mouse(&mdec);

	/* make memory management table */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	sprint(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	/* memory amount check */
	int i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
	sprint(s, "memory %dMB", i);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	init_palette();												// color palette settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_mouse = (unsigned char *) memman_alloc_4k(memman, 256);
	
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 透明色なし */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);		// screen initialization

	/* mouse cursor default*/
	int mx = (binfo->scrnx - 16) / 2; 
	int	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(buf_mouse, 99);

	sheet_slide(shtctl, sht_back, 0, 0);
	sheet_slide(shtctl, sht_mouse, mx, my);
	sheet_updown(shtctl, sht_back,  0);
	sheet_updown(shtctl, sht_mouse, 1);	

	sprint(s, "(%d, %d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	sprint(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);	
	
	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0){
			io_stihlt();
		}else{
			if(fifo8_status(&keyfifo) != 0){
				unsigned char data = fifo8_get(&keyfifo);	// unsigned ???
				char s[40];
				io_sti();
				sprint(s, "%x", data);
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);				
			}else if(fifo8_status(&mousefifo) != 0){
				unsigned char data = fifo8_get(&mousefifo);
				char s[40];
				io_sti();
				if (mouse_decode(&mdec, data) != 0) {
					sprint(s, "[lcr %d %d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);
					/* マウスカーソルの移動 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprint(s, "(%d, %d)", mx, my);
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 座標消す */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 座標書く */
					sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
					sheet_slide(shtctl, sht_mouse, mx, my);			
				}
			}
		}
	}
}
