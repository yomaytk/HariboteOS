/* ~~~~~ multiple task ~~~~~ */

#include<stdio.h>
#include "bootpack.h"
#include"mss_libc32.h"


struct TASKCTL *taskctl;
struct TIMER *task_timer;

/* return task running now */
struct TASK *task_now(){
	
	struct TASKLEVEL *tlev = &taskctl->level[taskctl->now_lv];
	return tlev->tasks[tlev->now];

}

/* add task to current tasklevel */
void task_add(struct TASK *task){

	struct TASKLEVEL *tlev = &taskctl->level[task->level];
	tlev->tasks[tlev->running] = task;
	tlev->running++;
	task->flags = 2;
	return;

}

/* remove task from current tasklevel */
void task_remove(struct TASK *task){

	int i;
	struct TASKLEVEL *tlev = &taskctl->level[task->level];

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
void task_switchsub(int id){

	for(int i = id;;i++){
		if(taskctl->level[i%MAX_TASKLEVELS].running > 0){
			taskctl->now_lv = i%MAX_TASKLEVELS;
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
	struct TASK *task, *idle_task;
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
	task_switchsub(0);
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);

	idle_task = task_alloc();
	idle_task->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle_task->tss.eip = (int) &task_idle;
	idle_task->tss.es = 1 * 8;
	idle_task->tss.cs = 2 * 8;
	idle_task->tss.ss = 1 * 8;
	idle_task->tss.ds = 1 * 8;
	idle_task->tss.fs = 1 * 8;
	idle_task->tss.gs = 1 * 8;
	task_run(idle_task, MAX_TASKLEVELS - 1, 1);

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
			task->tss.ss0 = 0;
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
	struct TASK *next_task, *now_task = tlev->tasks[tlev->now];
	
	tlev->now++;
	int flag = 0;
	if(tlev->now == tlev->running){
		tlev->now = 0;
		flag = 1;
	}
	if(taskctl->lv_change != 0){
		task_switchsub(0);
		tlev = &taskctl->level[taskctl->now_lv];
	}else if(flag == 1){
		task_switchsub(taskctl->now_lv+1);
		tlev = &taskctl->level[taskctl->now_lv];		
	}
	next_task = tlev->tasks[tlev->now];
	timer_settime(task_timer, next_task->priority);
	
	if (now_task != next_task) {
		farjmp(0, next_task->sel);
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
			task_switchsub(0);
			now_task = task_now();
			farjmp(0, now_task->sel);
		}
	}

	return;
}
