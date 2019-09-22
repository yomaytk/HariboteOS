/* ~~~~~ TSS method etc ~~~~~ */

#include<stdio.h>
#include"bootpack.h"

void task_b_main(struct SHEET *sht_back)
{
	struct FIFO32 fifo;
	struct TIMER *timer_ts, *timer_put;
	int data, fifobuf[128], count = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);
	timer_put = timer_alloc();
	timer_init(timer_put, &fifo, 1);
	timer_settime(timer_put, 1);

	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			data = fifo32_get(&fifo);
			io_sti();
			if (data == 1) {
				sprint(s, "%d", count);
				putfonts8_asc_sht(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, s, 11);
				timer_settime(timer_put, 1);
			} else if (data == 2) {
				farjmp(0, 3 * 8);
				timer_settime(timer_ts, 2);
			}
		}
	}
}
