/* ~~~~~ TSS method etc ~~~~~ */

#include<stdio.h>
#include"bootpack.h"

/* task_b */
void task_b_main(struct SHEET *sht_win_b)
{
	struct FIFO32 fifo;
	struct TIMER *timer_1s;
	int data, fifobuf[128], count = 0, count0 = 0;
	char s[20];

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for (;;) {
		count++;
		for(int data = 0;data < 1500;data++)	data = data;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();			// io_sti()???
		} else {
			data = fifo32_get(&fifo);
			io_sti();
			if(data == 100){
				sprint(s, "%d", count);
				putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				count = count0;			
				timer_settime(timer_1s, 100);
			}
		}
	}
}

/* idle_task */
void task_idle(){
	for(;;){
		io_hlt();
	}
}

/* console_task */
void console_task(struct SHEET *sht_cons)
{
	struct TIMER *timer;
	struct TASK *task = task_now();

	int fifobuf[128], cursor_x = 24, cursor_c = COL8_000000;
	fifo32_init(&task->fifo, 128, fifobuf, task);

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
						putfonts8_asc_sht(sht_cons, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				}else if (cursor_x < 240) {
					/* normal character */
					s[0] = data - 256;
					s[1] = 0;
					putfonts8_asc_sht(sht_cons, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
					cursor_x += 8;
				}
			}else if (data <= 1) { /* カーソル用タイマ */
				if (data != 0) {
					timer_init(timer, &task->fifo, 0); /* 次は0を */
					cursor_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1); /* 次は1を */
					cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);
				boxfill8(sht_cons->buf, sht_cons->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_cons, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}
