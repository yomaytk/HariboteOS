/* ~~~~~ exception process ~~~~~ */

#include<stdio.h>
#include<string.h>
#include"bootpack.h"
#include"mss_libc32.h"

int *inthandler0d(int *esp){

	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	struct TASK *task = task_now();
	char s[30];
	sprint(s, "EIP = %x\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);
	
}

int *inthandler0c(int *esp){

	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	cons_putstr0(cons, "\nINT 0D :\n Stack Exception.\n");
	struct TASK *task = task_now();
	char s[30];
	sprint(s, "EIP = %x\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);

}