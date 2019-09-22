/*===== main =====*/

#include<stdio.h>
#include"bootpack.h"


void main(){
	/* bootinformation */
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	/* buf for graphic */
	char s[100], mcursor[256], keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	/* FIFO32 */
	struct FIFO32 fifo;
	int fifobuf[128];
	/* memory */
	unsigned int memtotal = 0;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	/* sheet */
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, *buf_mouse, *buf_win;
	/* timer */
	struct TIMER *timer, *timer2, *timer3, *timer_ts;
	/* counter */
	unsigned int count = 0;
	/* keyboard */
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	int cursor_x = 8, cursor_c = COL8_FFFFFF;
	/* slide window */
	char sliding_flag = 0;
	int dx, dy;
	/* TSS */
	struct TSS32 tss_a, tss_b;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	init_gdtidt();			// GDT IDT initialization
	init_pic();				// PIC initialization
	io_sti(); 				/* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	init_pit();
	fifo32_init(&fifo, 128, fifobuf);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); 			// PIC1とキーボードを許可(11111000)
	io_out8(PIC1_IMR, 0xef); 			// マウスを許可(11101111)

	set490(&fifo, 1);
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);

	/* make memory management table */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	init_palette();												// color palette settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_mouse = (unsigned char *) memman_alloc_4k(memman, 256);
	buf_win = (unsigned char *)	memman_alloc_4k(memman, 160 * 52);
	
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 透明色なし */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);		// screen initialization
	make_window8(buf_win, 160, 52, "window");
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);

	/* mouse cursor default*/
	int mx = (binfo->scrnx - 16) / 2; 
	int	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(buf_mouse, 99);

	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);	

	sprint(s, "(%d, %d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	sprint(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	
	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	set_segmdesc(gdt + 3, 103, (int) &tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, (int) &tss_b, AR_TSS32);
	load_tr(3 * 8);
	int task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	*((int *) task_b_esp + 4) = (int) sht_back;		// to share sheet with other TSS
	tss_b.eip = (int) &task_b_main;
	tss_b.eflags = 0x00000202; /* IF = 1; */
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;

	sprint(s, "debug: %d", tss_b.esp);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 155, COL8_FFFFFF, s);	
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, binfo->scrny);


	for (;;) {

		for(int k = 0;k < 1100;k++);

		io_cli();
		if(fifo32_status(&fifo) == 0){
			io_stihlt();
		}else{
			int data = fifo32_get(&fifo);
			io_sti();
			if(data == 2){
				farjmp(0, 4 * 8);
				timer_settime(timer_ts, 2);
			}else if(256 <= data && data <= 511){
				char s[40];
				sprint(s, "%x", data - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (data == 256 + 0x0e && cursor_x > 8) { /* back space key */
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}else if (data < 0x54 + 256) {
					if (keytable[data - 256] != 0 && cursor_x < 144) { /* normal character */
						s[0] = keytable[data - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
					}
				}
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}else if(512 <= data && data <= 767){
				if (mouse_decode(&mdec, data-512) != 0) {
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
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);					
					/* マウスカーソルの移動 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sprint(s, "(%d, %d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);					
					sheet_slide(sht_mouse, mx, my);
					/* slide window by mouse */
					if ((mdec.btn & 0x01) != 0 && (sliding_flag == 1 || (sht_win->vx0 <= mx && 
							mx <= sht_win->vx0+sht_win->bxsize && sht_win->vy0 <= my && my <= sht_win->vy0+sht_win->bysize))) {
						if(sliding_flag == 0){
							sliding_flag = 1;
							dx = mx - sht_win->vx0;
							dy = my - sht_win->vy0;
						}
						sheet_slide(sht_win, mx - dx, my - dy);
					}else if(sliding_flag == 1){
						sliding_flag = 0;
					}
				}
			}else if(data == 10){
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);	
			}else if(data == 3){
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
			}else if (data <= 1) {
				if (data != 0) {
					timer_init(timer3, &fifo, 0);
					cursor_c = COL8_000000;
				} else {
					timer_init(timer3, &fifo, 1);
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}