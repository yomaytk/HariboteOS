[bits 32]

		global 	api_putchar
		global 	api_putstr0
		global	api_openwin
		global	api_putstrwin
		global 	api_boxfillwin
		global  api_initmalloc
		global  api_malloc
		global 	api_free
		global  api_point
		global 	api_refreshwin
		global 	api_linewin
		global 	api_closewin
		global 	api_getkey
		global  api_end

section .text

api_putchar:
		mov 	edx,1
		mov 	al,[esp+4]	; int c
		int 	0x40
		ret

api_putstr0:	; api_putstr0(char *s);
		push 	ebx
		mov 	ebx, [esp+8]
		mov 	edx, 2
		int 	0x40
		pop 	ebx
		ret

api_openwin:	; api_openwin(char *buf, int xsize, int ysize, int color, char *title)
		push 	ebx
		push 	esi
		push 	edi
		push 	ecx
		mov  	ebx, [esp+20]
		mov 	esi, [esp+24]
		mov 	edi, [esp+28]
		mov 	eax, [esp+32]
		mov		ecx, [esp+36]
		mov 	edx, 5
		int 	0x40
		pop 	ecx
		pop 	edi
		pop 	esi
		pop 	ebx
		ret

api_putstrwin:	; api_putstrwin(int win_num, int cur_x, int cur_y, int font_col, int str_size, char *str)
		pushad
		mov 	edx, 6
		mov 	ebx, [esp+36]
		mov 	esi, [esp+40]
		mov 	edi, [esp+44]
		mov 	eax, [esp+48]
		mov 	ecx, [esp+52]
		mov 	ebp, [esp+56]
		int 	0x40
		popad
		ret

api_boxfillwin:	; api_boxfillwin(int win_num, int x0, int y0, int x1, int y1, int box_col)
		pushad
		mov 	edx, 7
		mov 	ebx, [esp+36]
		mov 	eax, [esp+40]
		mov 	ecx, [esp+44]
		mov 	esi, [esp+48]
		mov 	edi, [esp+52]
		mov 	ebp, [esp+56]
		int 	0x40
		popad
		ret

api_initmalloc:
		push 	edx
		push 	ebx
		push 	eax
		push 	ecx
		mov 	edx, 8
		mov 	ebx, [cs:0x0020]
		mov 	eax, ebx
		add 	eax, 32*1024		; allock 32KB for struct memman on data segment
		mov 	ecx, [cs:0x0000]	; size data segment
		sub 	ecx, 32*1024
		int 	0x40
		pop 	ecx
		pop 	eax
		pop 	ebx
		pop  	edx
		ret

api_malloc:
		push 	edx
		push 	ebx
		push 	ecx
		mov 	edx, 9
		mov 	ebx, [cs:0x0020]
		mov 	ecx, [esp+16]
		int 	0x40
		pop 	edx
		pop 	ebx
		pop 	ecx
		ret

api_free:
		push 	edx
		push 	ebx
		push 	eax
		push 	ecx
		mov 	edx, 10
		mov 	ebx, [cs:0x0020]
		mov 	eax, [esp+20]		; start address for free area
		mov 	ecx, [esp+24]		; size of free area
		int 	0x40
		pop		edx
		pop 	ebx
		pop 	eax
		pop 	ecx
		ret

api_point:			; void api_point(int win, int x, int y, int col);
		push 	edx
		push 	ebx
		push 	esi
		push 	edi
		push 	eax
		mov 	edx, 11
		mov 	ebx, [esp+24]
		mov 	esi, [esp+28]
		mov 	edi, [esp+32]
		mov 	eax, [esp+36]
		int 	0x40
		pop 	eax
		pop 	edi
		pop 	esi
		pop 	ebx
		pop 	edx
		ret

api_refreshwin:
		push 	edi
		push 	esi
		push 	ebx
		mov 	edx, 12
		mov 	ebx, [esp+16]
		mov 	eax, [esp+20]
		mov 	ecx, [esp+24]
		mov 	esi, [esp+28]
		mov 	edi, [esp+32]
		int 	0x40
		pop		ebx
		pop 	esi
		pop 	edi
		ret

api_linewin:		; api_linewin(int win, int x0, int y0, int x1, int y1, int col)
		push 	edi
		push 	esi
		push 	ebp
		push 	ebx
		mov 	edx, 13
		mov 	ebx, [esp+20]
		mov 	eax, [esp+24]
		mov 	ecx, [esp+28]
		mov 	esi, [esp+32]
		mov 	edi, [esp+36]
		mov 	ebp, [esp+40]
		int 	0x40
		pop 	ebx
		pop 	ebp
		pop		esi
		pop 	edi
		ret

api_closewin:
		push 	ebx
		mov 	edx, 14
		mov 	ebx, [esp+8]
		int 	0x40
		pop 	ebx
		ret

api_getkey:
		mov 	edx, 15
		mov 	eax, [esp+4]
		int 	0x40
		ret

api_end:
		mov 	edx, 4
		int 	0x40