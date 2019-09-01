#include<stdio.h>
#include"bootpack.h"

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf){	// unsigned char *buf ???
	
	fifo->buf = buf;	
	fifo->p = 0;
	fifo->q = 0;
	fifo->size = size;
	fifo->free = size;
	fifo->flags = 0;
	
	return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data){	// unsigned char data ???
	
	if(fifo->free == 0){
		fifo->free |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[(fifo->p)++] = data;
	fifo->free--;
	if(fifo->p == fifo->size){
		fifo->p = 0;
	}
	return 1;
}

int fifo8_get(struct FIFO8 *fifo){
	
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

int fifo8_status(struct FIFO8 *fifo){
	return fifo->size - fifo->free;
}