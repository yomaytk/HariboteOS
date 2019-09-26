/*===== main =====*/

#include<stdio.h>
#include"bootpack.h"
#include"mss_libc32.h"


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
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, *buf_mouse, *buf_win, *buf_cons;
	/* cursor_timer */
	struct TIMER *cursor_timer;
	/* keyboard input */
	
	int cursor_x = 8, cursor_c = COL8_FFFFFF;
	/* slide window */
	char sliding_flag = 0;
	int dx, dy;
	/* task */
	struct TASK *task_a, *task_cons;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	extern struct TASKCTL *taskctl;
	/* switch active window */
	int key_to = 0;
	/* shift, CapsLock, NumLock, key_flag */
	int key_shift = 0, key_leds = (binfo->leds >> 4) & 7;	// bit4->ScrollLock, bit5->NumLock, bit6->CapsLock
	/* keyboard LED */
	int keycmd_wait = -1;
	struct FIFO32 keycmd;

	init_gdtidt();			// GDT IDT initialization
	init_pic();				// PIC initialization
	io_sti(); 				/* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	init_pit();
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); 			// PIC1とキーボードを許可(11111000)
	io_out8(PIC1_IMR, 0xef); 			// マウスを許可(11101111)

	/* make memory management table */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	init_palette();		// color palette settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	/* sht_back initialization */
	sht_back  = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 透明色なし */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	int mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	int my = (binfo->scrny - 28 - 16) / 2;

	/* sht_win initialization */
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); /* 透明色なし */
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	cursor_timer = timer_alloc();
	timer_init(cursor_timer, &fifo, 1);
	timer_settime(cursor_timer, 50);

	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* 透明色なし */
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_main;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2); /* level=2, priority=2 */
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons, 32, 4);
	sheet_slide(sht_win, 8, 56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons, 1);
	sheet_updown(sht_win, 2);
	sheet_updown(sht_mouse, 3);

	sprint(s, "debug: %d", sht_cons);
	putfonts8_asc_sht(sht_back, 850, 720, COL8_FFFFFF, COL8_008484, s, 15);
	
	/* 最初にキーボード状態との食い違いがないように、設定しておくことにする */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	for (;;) {
		
		/* keyboard LED data send */
		if(fifo32_status(&keycmd) > 0 && keycmd_wait < 0){
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		/* interruption process */
		io_cli();
		if(fifo32_status(&fifo) == 0){
			task_sleep(task_a);
			io_sti();
		}else{
			int data = fifo32_get(&fifo);
			io_sti();
			if(256 <= data && data <= 511){
				/* keyboard interupption */
				char s[40];
				if (data == 256 + 0x0e) { /* back space key */
					if(key_to == 0 && cursor_x > 8){
						/* task_a */
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
						cursor_x -= 8;
					}else{
						/* task_cons */
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}else if (data == 256 + 0x0f) { /* Tab */
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
						fifo32_put(&task_cons->fifo, 2);	/* cursor display ON on console */
					} else {
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;
						fifo32_put(&task_cons->fifo, 3);	/* cursor display OFF on cosole */
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}else if(data == 256 + 0x1c){
					if(key_to != 0){
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}else if (data == 256 + 0x2a) {	/* left shift key ON */
					key_shift |= 1;
				}else if (data == 256 + 0x36) {	/* right shift key ON */
					key_shift |= 2;
				}else if (data == 256 + 0xaa) {	/* left shift key OFF */
					key_shift &= ~1;
				}else if (data == 256 + 0xb6) {	/* right shift key OFF */
					key_shift &= ~2;
				}else if (data == 256 + 0x3a) {	/* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}else if (data == 256 + 0x45) {	/* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}else if (data == 256 + 0x46) {	/* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}else if (data == 256 + 0xfa) {	/* sending data to keyboard success */
					keycmd_wait = -1;
				}else if (data == 256 + 0xfe) {	/* sending data to keyboard fail */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}else if (data < 0x80 + 256) {
					/* normal character input */
					/* shift key ON ?*/
					if(key_shift == 0){
						s[0] = keytable0[data - 256];
					}else{
						s[0] = keytable1[data - 256];
					}
					/* CapsLock key OFF ?*/
					if('A' <= s[0] && s[0] <= 'Z'){
						if(((key_leds & 4) == 0 && (key_shift == 0)) || ((key_leds & 4) == 1 && (key_shift == 1))){
							s[0] += 0x20;
						}
					}
					s[1] = 0;
					if(key_to == 0){
						/* task_a */
						if (keytable0[data - 256] != 0 && cursor_x < 128) {
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					}else{
						/* task_cons */
						fifo32_put(&task_cons->fifo, s[0] + 256);						
					}
				}
			}else if(512 <= data && data <= 767){
				/* mouse interruption */
				if (mouse_decode(&mdec, data-512) != 0) {
					/* mouse cousor move */
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
			}else if (data <= 1) {
				if (data != 0) {
					timer_init(cursor_timer, &fifo, 0);
					if(cursor_c >= 0){
						cursor_c = COL8_000000;
					}
				} else {
					timer_init(cursor_timer, &fifo, 1);
					if(cursor_c >= 0){
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(cursor_timer, 50);
				/* cursor display again */
				if(cursor_c >= 0)	{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}