/* ~~~~~ TSS method etc ~~~~~ */

#include<stdio.h>
#include"bootpack.h"

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
		for(int i = 0;i < 1300;i++)	i = i;
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
