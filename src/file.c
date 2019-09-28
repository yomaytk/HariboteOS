/* ~~~~~ File load from disk ~~~~~ */

#include<stdio.h>
#include"bootpack.h"
#include"mss_libc32.h"

/* release FAT compression */
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

/* load file context */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}

struct FILEINFO *file_search(char cmdline[]){

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);

	char s[50];
	int x, y;

/* prepare filename */
	for (y = 0; y < 11; y++) {
		s[y] = ' ';
	}
	y = 0;
	for (x = 0; y < 11 && cmdline[x] != 0; x++) {
		if (cmdline[x] == '.' && y <= 8) {
			y = 8;
		} else {
			s[y] = cmdline[x];
			if ('a' <= s[y] && s[y] <= 'z') {
				/* convert capitals character */
				s[y] -= 0x20;
			}
			y++;
		}
	}
/* look for file */
	for (x = 0; x < 224 ;) {
		if (finfo[x].name[0] == 0x00) {
			break;
		}
		if ((finfo[x].type & 0x18) == 0) {
			for (y = 0; y < 11; y++) {
				if (finfo[x].name[y] != s[y]) {
					goto next_file;
				}
			}
			return finfo+x; /* file found */
		}
next_file:
		x++;
	}

	return 0;/* file cannot found */

}