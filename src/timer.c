/* ~~~~~ timer ~~~~~ */
#include<stdio.h>
#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* �m�ۂ������ */
#define TIMER_FLAGS_USING		2	/* �^�C�}�쓮�� */

void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.nexttime = 0xffffffff;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* ���g�p */
	}
	t = timer_alloc();
	t->next = 0;
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	timerctl.t0 = t;
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; /* ������Ȃ����� */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* ���g�p */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* insert top */
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.nexttime = timer->timeout;
		io_store_eflags(e);
		return;
	}
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			/* s -> timer -> t */
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
	return;
}

void inthandler20(int *esp)
{
	int i;
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00��t������PIC�ɒʒm */
	timerctl.count++;
	if (timerctl.nexttime > timerctl.count) {
		return;
	}
	timer = timerctl.t0;
	for (;;) {
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* timeout */
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next; /* ���̃^�C�}�̔Ԓn��timer�ɑ�� */
	}
	timerctl.t0 = timer;
	timerctl.nexttime = timerctl.t0->timeout;
	return;
}

void set490(struct FIFO32 *fifo, int mode)
{
	int i;
	struct TIMER *timer;
	if (mode != 0) {
		for (i = 0; i < 490; i++) {
			timer = timer_alloc();
			timer_init(timer, fifo, 1024 + i);
			timer_settime(timer, 100 * 60 * 60 * 24 * 50 + i * 100);
		}
	}
	return;
}
