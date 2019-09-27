/* ~~~~~ console task ~~~~~ */

#include<stdio.h>
#include<string.h>
#include"bootpack.h"
#include"mss_libc32.h"


/* console scroll */
int cons_newline(int cursor_y, struct SHEET *sht_cons){
	
	if (cursor_y < 28 + DEFAULT_CONS_YSIZE - 69) {
		cursor_y += 16;
	}else{
		/* scroll */
		/* overwrite lines except for last line*/
		for (int y = 28; y < 28 + DEFAULT_CONS_YSIZE - 53; y++) {
			for (int x = 8; x < 8 + DEFAULT_CONS_XSIZE - 16; x++) {
				sht_cons->buf[x + y * sht_cons->bxsize] = sht_cons->buf[x + (y + 16) * sht_cons->bxsize];
			}
		}
		/* overwrite last line by black */
		for (int y = 28 + DEFAULT_CONS_YSIZE - 53; y < 28 + DEFAULT_CONS_YSIZE - 37; y++) {
			for (int x = 8; x < 8 + DEFAULT_CONS_XSIZE - 16; x++) {
				sht_cons->buf[x + y * sht_cons->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sht_cons, 8, 28, 8 + DEFAULT_CONS_XSIZE - 16, 28 + DEFAULT_CONS_YSIZE - 37);
	}
	return cursor_y;
}

/* mem comamnd */
int mem(struct SHEET *sht_cons, unsigned int memtotal, int cursor_y){

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
int clear(struct SHEET *sht_cons){

	for (int y = 28; y < 28 + DEFAULT_CONS_YSIZE - 37; y++) {
		for (int x = 8; x < 8 + DEFAULT_CONS_XSIZE - 16; x++) {
			sht_cons->buf[x + y * sht_cons->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sht_cons, 8, 28, 8 + DEFAULT_CONS_XSIZE - 16, 28 + DEFAULT_CONS_XSIZE - 37);
	return 28;
}

/* ls command */
int ls(struct SHEET *sht_cons, int cursor_y){

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
				putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
				cursor_y = cons_newline(cursor_y, sht_cons);
			}
		}
	}
	cursor_y = cons_newline(cursor_y, sht_cons);

	return cursor_y;

}

/* release FAT compression */
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

/* load file context */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}

/* cat command */
int cat(struct SHEET *sht_cons, char cmdline[], int cursor_y, int *fat){

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[50];
	int x, y;

	/* prepare filename */
		for (y = 0; y < 11; y++) {
			s[y] = ' ';
		}
		y = 0;
		for (x = 4; y < 11 && cmdline[x] != 0; x++) {
			if (cmdline[x] == '.' && y <= 8) {
				y = 8;
			} else {
				s[y] = cmdline[x];
				if ('a' <= s[y] && s[y] <= 'z') {
					/* convert capitals character */
					s[y] -= 0x20;
				}
				y++;
			}
		}
		/* look for file */
		for (x = 0; x < 224 ;) {
			if (finfo[x].name[0] == 0x00) {
				break;
			}
			if ((finfo[x].type & 0x18) == 0) {
				for (y = 0; y < 11; y++) {
					if (finfo[x].name[y] != s[y]) {
						goto type_next_file;
					}
				}
				break;
			}
type_next_file:
			x++;
		}
		if (x < 224 && finfo[x].name[0] != 0x00) {
			/* case of finding file */
			char *p = (char *) memman_alloc_4k(memman, finfo[x].size);
			file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
			int cursor_x = 8;
			for (y = 0; y < finfo[x].size; y++) {
				s[0] = p[y];
				s[1] = 0;
				if (s[0] == 0x09) {	/* tab */
					for (;;) {
						putfonts8_asc_sht(sht_cons, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x += 8;
						if (cursor_x == 8 + DEFAULT_CONS_XSIZE - 16) {
							cursor_x = 8;
							cursor_y = cons_newline(cursor_y, sht_cons);
						}
						if (((cursor_x - 8) & 0x1f) == 0) {
							break;	/* if 0 mod 32, break */
						}
					}
				} else if (s[0] == 0x0a) {	/* new line */
					cursor_x = 8;
					cursor_y = cons_newline(cursor_y, sht_cons);
				} else if (s[0] == 0x0d) {	/* return */
					/* do nothing on current stage */
				} else {	/* normal character */
					putfonts8_asc_sht(sht_cons, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
					cursor_x += 8;
					if (cursor_x == 8 + DEFAULT_CONS_XSIZE - 16) {
						cursor_x = 8;
						cursor_y = cons_newline(cursor_y, sht_cons);
					}
				}
				memman_free_4k(memman, (int) p, finfo[x].size);
			}
		} else {
			/* case of cannot finding file */
			putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
			cursor_y = cons_newline(cursor_y, sht_cons);
		}
	cursor_y = cons_newline(cursor_y, sht_cons);
	return cursor_y;
}

/* execution hlt.bin */
int hlt_exe(struct SHEET *sht_cons, int cursor_y, int *fat){

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);	
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	int x, y;
	char s[] = "HLT     BIN";

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
			farjmp(0, 1003 * 8);
			memman_free_4k(memman, (int) p, finfo[x].size);
		} else {
			/* case of cannot finding file */
			putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
			cursor_y = cons_newline(cursor_y, sht_cons);
		}
	cursor_y = cons_newline(cursor_y, sht_cons);
	return cursor_y;
}

/* All command set */
int command_set(struct SHEET *sht_cons, char cmdline[], char cmd_size, unsigned int memtotal, int cursor_y, int *fat){

	if (strcomp(cmdline, "mem", cmd_size, 3) == 0){
		return mem(sht_cons, memtotal, cursor_y);
	}else if(strcomp(cmdline, "clear", cmd_size, 5) == 0){
		return clear(sht_cons);
	}else if(strcomp(cmdline, "ls", cmd_size, 2) == 0){
		return ls(sht_cons, cursor_y);
	}else if (strcomp(cmdline, "cat ", 4, 4) == 0) {
		return cat(sht_cons, cmdline, cursor_y, fat);
	}else if(strcomp(cmdline, "hlt", 3, 3) == 0){
		hlt_exe(sht_cons, cursor_y, fat);
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

	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);	/* store FAT data released compression */
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));	

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
						cmd_size--;
					}
				}else if(data == 10 + 256){
					/* enter key */
					/*  cursor delete */
					putfonts8_asc_sht(sht_cons, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 3] = 0;
					cursor_y = cons_newline(cursor_y, sht_cons);
					/* command */
					cursor_y = command_set(sht_cons, cmdline, cmd_size, memtotal, cursor_y, fat);
					/* pronpt display */
					putfonts8_asc_sht(sht_cons, 8, cursor_y, COL8_FFFFFF, COL8_000000, "$", 1);
					putfonts8_asc_sht(sht_cons, 16, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 24;
					cmd_size = 0;
				}else if (cursor_x < DEFAULT_CONS_XSIZE - 16) {
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
