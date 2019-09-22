/* ~~~~~ TSS method etc ~~~~~ */

#include<stdio.h>
#include"bootpack.h"

void task_b_main(struct SHEET *sht_back)
{
	struct FIFO32 fifo;
	struct TIMER *timer_put, *timer_1s;
	int data, fifobuf[128], count = 0, count1s = 0;
	char s[20];

	fifo32_init(&fifo, 128, fifobuf);
	// timer_put = timer_alloc();
	// timer_init(timer_put, &fifo, 1);
	// timer_settime(timer_put, 1);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);
	
	for (;;) {
		count++;
		for(int i = 0;i < 1600;i++)	i = i;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();			// io_sti()???
		} else {
			data = fifo32_get(&fifo);
			io_sti();
			sprint(s, "%d", count - count1s);
			putfonts8_asc_sht(sht_back, 150, 128, COL8_FFFFFF, COL8_008484, s, 11);				
			count1s = count;
			timer_settime(timer_1s, 100);
		}
	}
}
