#include<stdio.h>

int strcomp(char ch[], char ch_tag[], int size, int size2){
	int i;

	for(i = 0;i < size;i++){
		if(ch[i] > ch_tag[i]){
			return i;
		}else if(ch[i] < ch_tag[i]){
			return -1;
		}
	}
	if(size == size2)	return 0;
	else 	return -1;
}
int main(){
	// int *p;
	// printf("%d\n", p);
	// // printf("%d\n", q);
	// int *q;
	// // p = (int *) 4;
	// int i = 5;
	// p = &i;
	// // printf("%d\n", *p);
	// printf("%d\n", p);
	// printf("%d\n", &i);
	// printf("%d\n", q);
	char s[3], ss[5];
	s[0] = 'm';
	s[1] = 'e';
	s[2] = 'm';
	ss[0] = 'a';
	ss[1] = 'b';
	ss[2] = 'c';
	int a = strcomp(s, ss, 3, 4);
	printf("%d", a);
	// printf("%d", s[3]);
	// char aa[10];
	// aa[0] = 'a';
	// aa[1] = 'b';
	// for(int i = 0;i < 11;i++)	printf("%d\n", aa[i]);
}