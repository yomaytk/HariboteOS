/* ~~~~~ library 32bit C ~~~~~ */

#include<stdio.h>
#include<stdarg.h>
#include"mss_libc32.h"


/*===== sprint‚Ì•â•ŠÖ” =====*/
int sub_sp(char *s, int arg, int dig){
	
	int a = arg, d = 0, f = 0, b;

	if(a < 0){
		*(s++) = 0x2d;	// 0x2d -> '-'
		a *= -1;
		f = 1;
	}
	b = a;	
	while(b > 0){b /= dig;d++;}
	if(d == 0)	d = 1;
	for(int j = d-1;j >= 0;j--){
		char c = a%dig;
		if(c < 10)	*(s+j) = 0x30+c;
		else 	*(s+j) = 0x60+(c-9);
		a /= dig;
	}
	return (d+f);
}
/*===== sprintf‚Ì‘ã‘ÖŠÖ” =====*/
void sprint(char *s, char *ss, ...){

	va_list itr;
	va_start(itr, *ss);
	int d = 0;

	while(*ss){
		if(*ss != '%' || *(ss+1) == 0x00){
			*(s++) = *(ss++);
		}else {
			char c = *(ss+1);
			if(c == 'd'){
				d = sub_sp(s, va_arg(itr, int), 10);
			}else if(c == 'x'){
				d = sub_sp(s, va_arg(itr, int), 16);
			}
			s += d;
			ss += 2;
		}
	}
	*s = 0x00;
	va_end(itr);
	return ;
}

/* char compare method */
int strcomp(char ch[], char ch_tag[], int size, int size2){
	int i;

	for(i = 0;i < size;i++){
		if(ch[i] > ch_tag[i]){
			return 1;
		}else if(ch[i] < ch_tag[i]){
			return -1;
		}
	}
	if(size == size2)	return 0;
	else 	return -1;
}