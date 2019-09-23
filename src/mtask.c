/* ~~~~~ multiple task ~~~~~ */

#include<stdio.h>
#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

/* return task running now */
struct TASK *task_now(){
	
	struct TASKLEVEL *tlev = &taskctl->level[taskctl->now_lv];
	return tlev->tasks[tlev->now];

}

/* add task to current tasklevel */
void task_add(struct TASK *task){

	struct TASKLEVEL *tlev = &taskctl->level[taskctl->now_lv];
	tlev->tasks[tlev->running] = task;
	tlev->running++;
	task->flags = 2;
	return;

}

/* remove task from current tasklevel */
void task_remove(struct TASK *task){

	int i;
	struct TASKLEVEL *tlev = &taskctl->level[taskctl->now_lv];

	/* find target task from tlev */
	for(i = 0;i < tlev->running;i++){
		if(tlev->tasks[i] == task){
			break;
		}
	}
	
	tlev->running--;
	if(i < tlev->now){
		tlev->now--;
	}
	if(tlev->now >= tlev->running){
		tlev->now = 0;
	}
	/* remove task by sliding task */
	for(;i < tlev->running;i++){
		tlev->tasks[i] = tlev->tasks[i+1];
	}
	task->flags = 1;	/* sleeping */

	return;
}

/* decide next tasklevel when task_switch */
void task_switchsub(){

	for(int i = 0;i < MAX_TASKLEVELS;i++){
		if(taskctl->level[i].running > 0){
			taskctl->now_lv = i;
			taskctl->lv_change = 0;
			break;
		}
	}

	return;

}

/* taskctl init */
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;	/* sel initialize! */
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; /* running */
	task->priority = 2;	/* default interval */
	task->level = 0;
	task_add(task);
	task_switchsub();
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; /* 使用中マーク */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* とりあえず0にしておくことにする */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /* all used */
}


void task_run(struct TASK *task, int level, int priority)
{
	if(level < 0){
		level = task->level;
	}
	if(priority > 0){
		task->priority = priority;
	}
	if(task->level != level && task->flags == 2){
		task_remove(task);
	}
	if(task->flags != 2){
		task->level = level; /* running mark */
		task_add(task);
	}

	taskctl->lv_change = 1;

	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tlev = &taskctl->level[taskctl->now_lv];
	struct TASK *task_next, *now_task = tlev->tasks[tlev->now];
	
	tlev->now++;
	if(tlev->now == tlev->running){
		tlev->now = 0;
	}
	if(taskctl->lv_change != 0){
		task_switchsub();
		task_next = task_now();
	}else{
		task_next = tlev->tasks[tlev->now];
	}
	
	timer_settime(task_timer, task_next->priority);
	
	if (now_task != task_next) {
		farjmp(0, task_next->sel);
	}
	return;
}

void task_sleep(struct TASK *task){
	
	struct TASK *now_task;
	int i;
	if(task->flags == 2){
		now_task = task_now();
		task_remove(task);
		if(now_task == task){
			task_switchsub();
			now_task = task_now();
			farjmp(0, now_task->sel);
		}
	}

	return;
}
