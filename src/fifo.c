/* ~~~~~ FIFO32 process ~~~~~ */

#include<stdio.h>
#include"bootpack.h"
#include"mss_libc32.h"


void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task){	// unsigned char *buf ???
	
	fifo->buf = buf;	
	fifo->p = 0;
	fifo->q = 0;
	fifo->size = size;
	fifo->free = size;
	fifo->flags = 0;
	fifo->task = task;
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data){	// unsigned char data ???
	
	if(fifo->free == 0){
		fifo->free |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[(fifo->p)++] = data;
	fifo->free--;
	if(fifo->p == fifo->size){
		fifo->p = 0;
	}
	if(fifo->task != 0){
		if(fifo->task->flags != 2){
			task_run(fifo->task, -1, 0);
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo){
	
	if(fifo->free == fifo->size){
		return -1;
	}
	int data = fifo->buf[(fifo->q)++];
	fifo->free++;
	if(fifo->q == fifo->size){
		fifo->q = 0;
	}
	return data;
}

int fifo32_status(struct FIFO32 *fifo){
	return fifo->size - fifo->free;
}