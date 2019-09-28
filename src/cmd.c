/* ~~~~~ console task ~~~~~ */

#include<stdio.h>
#include<string.h>
#include"bootpack.h"
#include"mss_libc32.h"


void cons_putchar(struct CONSOLE *cons, int ch, int move){

	char s[2];
	s[0] = ch;
	s[1] = 0;

	if (s[0] == 0x09) {	/* tab */
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + DEFAULT_CONS_XSIZE - 16) {
				cons->cur_x = 24;
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	/* if 0 mod 32, break */
			}
		}
	} else if (s[0] == 0x0a) {	/* new line */
		cons->cur_x = 24;
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* return */
		/* do nothing on current stage */
	} else {	/* normal character */
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if(move != 0){
			cons->cur_x += 8;
			if (cons->cur_x == 8 + DEFAULT_CONS_XSIZE - 24) {
				cons->cur_x = 8;
				cons_newline(cons);
			}
		}
	}
}

/* console scroll */
void cons_newline(struct CONSOLE *cons){
	
	if (cons->cur_y < 28 + DEFAULT_CONS_YSIZE - 69) {
		cons->cur_y += 16;
		cons->cur_x = 8;
	}else{
		/* scroll */
		/* overwrite lines except for last line*/
		for (int y = 28; y < 28 + DEFAULT_CONS_YSIZE - 53; y++) {
			for (int x = 8; x < 8 + DEFAULT_CONS_XSIZE - 16; x++) {
				cons->sht->buf[x + y * cons->sht->bxsize] = cons->sht->buf[x + (y + 16) * cons->sht->bxsize];
			}
		}
		/* overwrite last line by black */
		for (int y = 28 + DEFAULT_CONS_YSIZE - 53; y < 28 + DEFAULT_CONS_YSIZE - 37; y++) {
			for (int x = 8; x < 8 + DEFAULT_CONS_XSIZE - 16; x++) {
				cons->sht->buf[x + y * cons->sht->bxsize] = COL8_000000;
			}
		}
		cons->cur_x = 8;
		sheet_refresh(cons->sht, 8, 28, 8 + DEFAULT_CONS_XSIZE - 16, 28 + DEFAULT_CONS_YSIZE - 37);
	}
}

/* mem comamnd */
void mem(struct CONSOLE *cons, unsigned int memtotal){

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[50];

	sprint(s, "total   %dMB", memtotal / (1024 * 1024));
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(cons);
	sprint(s, "free %dKB", memman_total(memman) / 1024);
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(cons);
	cons_newline(cons);

}

/* clear command */
void clear(struct CONSOLE *cons){

	for (int y = 28; y < 28 + DEFAULT_CONS_YSIZE - 37; y++) {
		for (int x = 8; x < 8 + DEFAULT_CONS_XSIZE - 16; x++) {
			cons->sht->buf[x + y * cons->sht->bxsize] = COL8_000000;
		}
	}
	cons->cur_x = 8;
	cons->cur_y = 28;
	sheet_refresh(cons->sht, 8, 28, 8 + DEFAULT_CONS_XSIZE - 16, 28 + DEFAULT_CONS_XSIZE - 37);
}

/* ls command */
void ls(struct CONSOLE *cons){

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	char s[50];

	for (int x = 0; x < 224; x++) {
		if (finfo[x].name[0] == 0x00) {
			break;
		}
		if (finfo[x].name[0] != 0xe5) {
			if ((finfo[x].type & 0x18) == 0) {
				sprint(s, "filename.ext   %d", finfo[x].size);
				for (int y = 0; y < 8; y++) {
					s[y] = finfo[x].name[y];
				}
				s[9] = finfo[x].ext[0];
				s[10] = finfo[x].ext[1];
				s[11] = finfo[x].ext[2];
				putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
				cons_newline(cons);
			}
		}
	}
	cons_newline(cons);
}

/* cat command */
void cat(struct CONSOLE *cons, char cmdline[], int *fat){

	struct FILEINFO *finfo = file_search(cmdline);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[2];

	if (finfo != 0) {
		/* case of finding file */
		char *p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons->cur_x = 8;
		for (int y = 0; y < finfo->size; y++) {
			s[0] = p[y];
			s[1] = 0;
			cons_putchar(cons, (int) s[0], 1);
			memman_free_4k(memman, (int) p, finfo->size);
		}
	} else {
		/* case of cannot finding file */
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
		cons_newline(cons);
	}
	cons_newline(cons);
}

/* execution hlt.bin */
void sample1_exe(struct CONSOLE *cons, int *fat){

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);	
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	int x, y;
	char s[] = "SAMPLE1 BIN";

		for (x = 0; x < 224 ;) {
			if (finfo[x].name[0] == 0x00) {
				break;
			}
			if ((finfo[x].type & 0x18) == 0) {
				for (y = 0; y < 11; y++) {
					if (finfo[x].name[y] != s[y]) {
						goto hlt_next_file;
					}
				}
				break;
			}
hlt_next_file:
			x++;
		}
		if (x < 224 && finfo[x].name[0] != 0x00) {
			/* case of finding file */
			char *p = (char *) memman_alloc_4k(memman, finfo[x].size);
			file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
			set_segmdesc(gdt + 1003, finfo[x].size - 1, (int) p, AR_CODE32_ER);
			farcall(0, 1003 * 8);
			memman_free_4k(memman, (int) p, finfo[x].size);
		} else {
			/* case of cannot finding file */
			putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
			cons_newline(cons);
		}
	cons_newline(cons);
}

/* All command set */
void command_set(struct CONSOLE *cons, char cmdline[], char cmd_size, unsigned int memtotal, int *fat){

	if (strcomp(cmdline, "mem", cmd_size, 3) == 0){
		mem(cons, memtotal);
	}else if(strcomp(cmdline, "clear", cmd_size, 5) == 0){
		clear(cons);
	}else if(strcomp(cmdline, "ls", cmd_size, 2) == 0){
		ls(cons);
	}else if (strcomp(cmdline, "cat ", 4, 4) == 0) {
		cat(cons, cmdline, fat);
	}else if(strcomp(cmdline, "hlt", 3, 3) == 0){
		sample1_exe(cons, fat);
	}else if (cmdline[0] != 0) {
		/* not command or empty line */
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
		cons_newline(cons);
		cons_newline(cons);
	}
}

/* console_main */
void console_main(struct SHEET *sht_cons, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	/* console */
	struct CONSOLE cons;
	cons.sht = sht_cons;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int *) 0x0fec) = (int) &cons;

	/* interruption buffer of keyboard, mouse, timer etc...*/
	int fifobuf[128];
	fifo32_init(&task->fifo, 128, fifobuf, task);
	char cmdline[100], cmd_size = 0;

	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);	/* store FAT data released compression */
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));	

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	cons_putchar(&cons, '$', 1);
	cons_putchar(&cons, '>', 1);

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
					if(cons.cur_x > 24){
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
						cmd_size--;
					}
				}else if(data == 10 + 256){
					/* enter key */
					/*  cursor delete */
					cons_putchar(&cons, ' ', 1);					
					cmdline[cons.cur_x / 8 - 3] = 0;
					cons_newline(&cons);
					/* command */
					command_set(&cons, cmdline, cmd_size, memtotal, fat);
					/* pronpt display */
					cons_putchar(&cons, '$', 1);
					cons_putchar(&cons, '>', 1);
					cons.cur_x = 24;
					cmd_size = 0;
				}else if (cons.cur_x < DEFAULT_CONS_XSIZE - 16) {
					/* normal character */
					s[0] = data - 256;
					s[1] = 0;
					cmdline[cons.cur_x/8 - 3] = s[0];
					cmd_size++;
					cons_putchar(&cons, s[0], 1);
				}
			}else if(data == 2){
				cons.cur_c = COL8_000000;
			}else if(data == 3){
				cons.cur_c = -1;
				boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, 43);
			}else if (data <= 1) { /* cursor timer */
				if (data != 0) {
					timer_init(timer, &task->fifo, 0); /* next 0 */
					if(cons.cur_c >= 0)	cons.cur_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1); /* next 1 */
					if(cons.cur_c >= 0)	cons.cur_c = COL8_000000;
				}
				timer_settime(timer, 50);
				/* cursor display */
				if(cons.cur_c >= 0){
					boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
			}
		}
	}
}
