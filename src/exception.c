/* ~~~~~ exception process ~~~~~ */

#include<stdio.h>
#include<string.h>
#include"bootpack.h"
#include"mss_libc32.h"

int inthandler0d(int *esp){

	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	return 1;
	
}