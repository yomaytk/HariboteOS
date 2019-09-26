/* ~~~~~ console task ~~~~~ */

#include<stdio.h>
#include<string.h>
#include"bootpack.h"
#include"mss_libc32.h"


/* console scroll */
int cons_newline(int cursor_y, struct SHEET *sht_cons){
	
	if (cursor_y < 28 + 112) {
		cursor_y += 16;
	}else{
		/* scroll */
		/* overwrite lines except for last line*/
		for (int y = 28; y < 28 + 112; y++) {
			for (int x = 8; x < 8 + 240; x++) {
				sht_cons->buf[x + y * sht_cons->bxsize] = sht_cons->buf[x + (y + 16) * sht_cons->bxsize];
			}
		}
		/* overwrite last line by black */
		for (int y = 28 + 112; y < 28 + 128; y++) {
			for (int x = 8; x < 8 + 240; x++) {
				sht_cons->buf[x + y * sht_cons->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sht_cons, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}

/* mem comamnd */
int mem(struct SHEET *sht_cons, char cmdline[], char cmd_size, unsigned int memtotal, int cursor_y){

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[50];

	sprint(s, "total   %dMB", memtotal / (1024 * 1024));
	putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
	cursor_y = cons_newline(cursor_y, sht_cons);
	sprint(s, "free %dKB", memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
	cursor_y = cons_newline(cursor_y, sht_cons);
	cursor_y = cons_newline(cursor_y, sht_cons);
	
	return cursor_y;
}

/* clear command */
int clear(struct SHEET *sht_cons, char cmdline[], char cmd_size){

	for (int y = 28; y < 28 + 128; y++) {
		for (int x = 8; x < 8 + 240; x++) {
			sht_cons->buf[x + y * sht_cons->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sht_cons, 8, 28, 8 + 240, 28 + 128);
	return 28;
}

int command_set(struct SHEET *sht_cons, char cmdline[], char cmd_size, unsigned int memtotal, int cursor_y){

	if (strcomp(cmdline, "mem", cmd_size, 3) == 0){
		return mem(sht_cons, cmdline, cmd_size, memtotal, cursor_y);
	}else if(strcomp(cmdline, "clear", cmd_size, 5) == 0){
		return clear(sht_cons, cmdline, cmd_size);
	}else if (cmdline[0] != 0) {
		/* not command or empty line */
		putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
		cursor_y = cons_newline(cursor_y, sht_cons);
		cursor_y = cons_newline(cursor_y, sht_cons);
		return cursor_y;
	}
	return cursor_y;	/* error */
}
/* console_main */
void console_main(struct SHEET *sht_cons, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	int fifobuf[128], cursor_x = 24, cursor_y = 28, cursor_c = -1;
	fifo32_init(&task->fifo, 128, fifobuf, task);
	char cmdline[100], cmd_size = 0;

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	putfonts8_asc_sht(sht_cons, 8, 28, COL8_FFFFFF, COL8_000000, "$", 1);
	putfonts8_asc_sht(sht_cons, 16, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			int data;
			data = fifo32_get(&task->fifo);
			io_sti();
			char s[10];
			if(256 <= data && data <= 512){
				if (data == 8 + 256) {
					/* back space key */
					if(cursor_x > 24){
						putfonts8_asc_sht(sht_cons, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				}else if(data == 10 + 256){
					/* enter key */
					/*  cursor delete */
					putfonts8_asc_sht(sht_cons, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 3] = 0;
					cursor_y = cons_newline(cursor_y, sht_cons);
					/* command */
					cursor_y = command_set(sht_cons, cmdline, cmd_size, memtotal, cursor_y);
					/* pronpt display */
					putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, "$", 1);
					putfonts8_asc_sht(sht_cons, 16, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 24;
					cmd_size = 0;
				}else if (cursor_x < 240) {
					/* normal character */
					s[0] = data - 256;
					s[1] = 0;
					cmdline[cursor_x/8 - 3] = s[0];
					cmd_size++;
					putfonts8_asc_sht(sht_cons, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
					cursor_x += 8;
				}
			}else if(data == 2){
				cursor_c = COL8_000000;
			}else if(data == 3){
				cursor_c = -1;
				boxfill8(sht_cons->buf, sht_cons->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, 43);
				sheet_refresh(sht_cons, cursor_x, cursor_y, cursor_x + 8, 44);
			}else if (data <= 1) { /* cursor timer */
				if (data != 0) {
					timer_init(timer, &task->fifo, 0); /* next 0 */
					if(cursor_c >= 0)	cursor_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1); /* next 1 */
					if(cursor_c >= 0)	cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);
				if(cursor_c >= 0){
					boxfill8(sht_cons->buf, sht_cons->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
					sheet_refresh(sht_cons, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
				}
			}
		}
	}
}
