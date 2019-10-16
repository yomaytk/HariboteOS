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

/* String display */
void cons_putstr0(struct CONSOLE *cons, char *s){

	for(; *s != 0;s++){
		cons_putchar(cons, *s, 1);
	}

	return;
}

/* String display */
void cons_putstr1(struct CONSOLE *cons, char *s, int len){

	for(int i = 0;i < len;i++){
		cons_putchar(cons, s[i], 1);
	}

	return;
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

	return;
}

/* mem comamnd */
void mem(struct CONSOLE *cons, unsigned int memtotal){

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[50];

	sprint(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);

	return;
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
				cons_putstr0(cons, s);
				cons_newline(cons);
			}
		}
	}
	cons_newline(cons);
}

/* cat command */
void cat(struct CONSOLE *cons, char cmdline[], int *fat){

	struct FILEINFO *finfo = file_search(&cmdline[4]);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[2];

	if (finfo != 0) {
		/* case of finding file */
		char *p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons->cur_x = 8;
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		/* case of cannot finding file */
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);
}

void os_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
	int i, x, y, len, dx, dy;

	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0) {
		dx = - dx;
	}
	if (dy < 0) {
		dy = - dy;
	}
	if (dx >= dy) {
		len = dx + 1;
		if (x0 > x1) {
			dx = -1024;
		} else {
			dx =  1024;
		}
		if (y0 <= y1) {
			dy = ((y1 - y0 + 1) << 10) / len;
		} else {
			dy = ((y1 - y0 - 1) << 10) / len;
		}
	} else {
		len = dy + 1;
		if (y0 > y1) {
			dy = -1024;
		} else {
			dy =  1024;
		}
		if (x0 <= x1) {
			dx = ((x1 - x0 + 1) << 10) / len;
		} else {
			dx = ((x1 - x0 - 1) << 10) / len;
		}
	}

	for (i = 0; i < len; i++) {
		sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
		x += dx;
		y += dy;
	}

	return;
}


int app_exe(struct CONSOLE *cons, int *fat, char cmdline[], int cmdsize){

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;	
	struct TASK *task = task_now();
	struct SHTCTL *shtctl;

	int i = cmdsize;
	char *name = cmdline;

	name[i] = 0;
	
	finfo = file_search(name);

	/* add extension to name */
	if(finfo == 0){
		name[i] = '.';
		name[i+1] = 'b';
		name[i+2] = 'i';
		name[i+3] = 'n';
		finfo = file_search(name);	//	again loof for file
	}

	if (finfo != 0) {
		/* can find file */
		char *p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);			
		if(finfo->size >= 8 && strcomp((p+4), "main", 4, 4) == 0 && *p == 0x00){
			int segment_size = *((int *) (p + 0x0000));	/* data segment size of application */
			int data_start = *((int *) (p + 0x0014));		/* data start address of application */
			int esp = *((int *) (p + 0x000c));			/* start address of stack and assigned data start address */
			int data_size = *((int *) (p + 0x0010));
			char *q = (char *) memman_alloc_4k(memman, segment_size);
			*((int *) 0x0fe8) = (int) q;
			set_segmdesc(gdt + 1004, segment_size - 1, (int) q, AR_DATA32_RW + 0x60);			
			for(int i = 0;i < data_size;i++){
				q[esp + i] = p[data_start + i];
			}
			start_app(0x1b, 1003*8, esp, 1004*8, &(task->tss.esp0));
			/* if application window open, close it. */
			shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
			for(int i = 0;i < MAX_SHEETS;i++){
				struct SHEET *sht = &(shtctl->sheets0[i]);
				if(sht->flags != 0 && sht->task == task){
					sheet_free(sht);
				}
			}
			memman_free_4k(memman, (int) q, segment_size);
		}else{
			// cons_putstr0(cons, "*.o or *.bin file format error.\n");
			char *q = (char *) memman_alloc_4k(memman, 16*1024);
			*((int *) 0xfe8) = (int) p;
			set_segmdesc(gdt + 1004, 16*1024 - 1, (int) q, AR_DATA32_RW + 0x60);						
			start_app(0x00, 1003*8, 16*1024, 1004*8, &(task->tss.esp0));
			memman_free_4k(memman, (int) q, 64*1024);
		}
		memman_free_4k(memman, (int) p, finfo->size);
		cons_newline(cons);
		return 1;
	}

	return 0;
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
	}else if (cmdline[0] != 0) {
		/* not command or empty line */
		if(app_exe(cons, fat, cmdline, cmd_size) == 0){
			cons_putstr0(cons, "Bad command\n\n");
		}
	}

}

int *os_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
	
	int ds_base = *((int *) 0x0fe8);
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	struct TASK *task = task_now();
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht;
	int *reg = &eax + 1;	/* start address of register be using for saving register on stack */

	if(edx == 1){
		cons_putchar(cons, eax & 0xff, 1);
	}else if(edx == 2){
		cons_putstr0(cons, (char *) ebx + ds_base);
	}else if(edx == 3){
		cons_putstr1(cons, (char *) ebx + ds_base, ecx);
	}else if(edx == 4){
		return &(task->tss.esp0);
	}else if(edx == 5){
		sht = sheet_alloc(shtctl);
		sht->task = task;
		sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
		make_window8((char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
		sheet_slide(sht, 100, 50);
		sheet_updown(sht, 3);
		reg[7] = (int) sht;	/* eax register */
	}else if (edx == 6) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *) ebp + ds_base);
		if((ebx & 1) == 0){
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
	} else if (edx == 7) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
		if((ebx & 1) == 0){
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}else if(edx == 8){
		memman_init((struct MEMMAN *) (ebx + ds_base));
		ecx &= 0xfffffff0;
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
	}else if(edx == 9){
		ecx &= 0xfffffff0;
		reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
	}else if(edx == 10){
		ecx &= 0xfffffff0;
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
	}else if(edx == 11){
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		sht->buf[sht->bxsize * edi + esi] = eax;
		if((ebx & 1) == 0){
			sheet_refresh(sht, esi, edi, esi+1, edi+1);
		}
	}else if(edx == 12){
		sht = (struct SHEET *) ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
	}else if(edx == 13){
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		os_api_linewin(sht, eax, ecx, esi, edi, ebp);
		if(ebx & 1 == 0){
			sheet_refresh(sht, eax, ecx, esi+1, edi+1);
		}
	}else if(edx == 14){
		sheet_free((struct SHEET *) ebx);
	}else if(edx == 15){
		for(;;){
			io_cli();
			if(fifo32_status(&task->fifo) == 0){
				if(eax != 0){
					task_sleep(task);
				/* Why need ??? */
				}else{
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			int data = fifo32_get(&task->fifo);
			io_sti();
			if(data <= 1){
				timer_init(cons->timer, &task->fifo, 1);	/* not display cursor while app execution */
				timer_settime(cons->timer, 50);
			}else if(data == 2){
				cons->cur_c = COL8_FFFFFF;
			}else if(data == 3){
				cons->cur_c = -1;
			}else if(256 <= data && data <= 511){
				reg[7] = data - 256;
				return 0;
			}
		}
	}

	return 0;
}


/* console_main */
void console_main(struct SHEET *sht_cons, unsigned int memtotal)
{
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	/* console */
	struct CONSOLE cons;
	struct TIMER *timer = cons.timer;
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
