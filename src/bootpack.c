/*===== main =====*/

#include<stdio.h>
#include"bootpack.h"


void main()
{
	/* bootinformation */
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	/* buf for graphic */
	char s[100], mcursor[256], keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	/* memory */
	unsigned int memtotal, count = 0;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	/* sheet */
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, *buf_mouse, *buf_win;
	/* timer */
	struct FIFO8 timerfifo, timerfifo2, timerfifo3;
	char timerbuf[8], timerbuf2[8], timerbuf3[8];
	struct TIMER *timer, *timer2, *timer3;



	init_gdtidt();			// GDT IDT initialization
	init_pic();				// PIC initialization
	io_sti(); 				/* IDT/PIC�̏��������I������̂�CPU�̊��荞�݋֎~������ */
	init_pit();
	fifo8_init(&keyfifo, 32, keybuf);				// FIFO keyboard initialization
	fifo8_init(&mousefifo, 128, mousebuf);			// FIFO mouse initialization
	io_out8(PIC0_IMR, 0xf8); 			// PIC1�ƃL�[�{�[�h������(11111000)
	io_out8(PIC1_IMR, 0xef); 			// �}�E�X������(11101111)

	init_keyboard();
	enable_mouse(&mdec);

	fifo8_init(&timerfifo, 8, timerbuf);
	timer = timer_alloc();
	timer_init(timer, &timerfifo, 1);
	timer_settime(timer, 1000);
	fifo8_init(&timerfifo2, 8, timerbuf2);
	timer2 = timer_alloc();
	timer_init(timer2, &timerfifo2, 1);
	timer_settime(timer2, 300);
	fifo8_init(&timerfifo3, 8, timerbuf3);
	timer3 = timer_alloc();
	timer_init(timer3, &timerfifo3, 1);
	timer_settime(timer3, 50);

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
	
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* �����F�Ȃ� */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);		// screen initialization
	make_window8(buf_win, 160, 52, "counter");

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
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);	
	
	for (;;) {

		sprint(s, "%d", timerctl.count);
		boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);

		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) + fifo8_status(&timerfifo) 
			+ fifo8_status(&timerfifo2) + fifo8_status(&timerfifo3) == 0){
			io_sti();
		}else{
			if(fifo8_status(&keyfifo) != 0){
				unsigned char data = fifo8_get(&keyfifo);	// unsigned ???
				char s[40];
				io_sti();
				sprint(s, "%x", data);
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(sht_back, 0, 16, 16, 32);				
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
					sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);
					/* �}�E�X�J�[�\���̈ړ� */
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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* ���W���� */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* ���W���� */
					sheet_refresh(sht_back, 0, 0, 80, 16);
					sheet_slide(sht_mouse, mx, my);			
				}
			}else if (fifo8_status(&timerfifo) != 0) {
				unsigned int data = fifo8_get(&timerfifo); /* �Ƃ肠�����ǂݍ��ށi����ɂ��邽�߂Ɂj */
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 64, COL8_FFFFFF, "10[sec]");
				sheet_refresh(sht_back, 0, 64, 56, 80);
			}else if (fifo8_status(&timerfifo2) != 0) {
				unsigned int data = fifo8_get(&timerfifo2); /* �Ƃ肠�����ǂݍ��ށi����ɂ��邽�߂Ɂj */
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 80, COL8_FFFFFF, "3[sec]");
				sheet_refresh(sht_back, 0, 80, 48, 96);
			}else if (fifo8_status(&timerfifo3) != 0) {
				unsigned data = fifo8_get(&timerfifo3);
				io_sti();
				if (data != 0) {
					timer_init(timer3, &timerfifo3, 0); /* ����0�� */
					boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
				} else {
					timer_init(timer3, &timerfifo3, 1); /* ����1�� */
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
				}
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}
