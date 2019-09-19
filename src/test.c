#include<stdio.h>

int main(){
	int *p;
	printf("%d\n", p);
	// printf("%d\n", q);
	int *q;
	// p = (int *) 4;
	int i = 5;
	p = &i;
	// printf("%d\n", *p);
	printf("%d\n", p);
	printf("%d\n", &i);
	printf("%d\n", q);
}