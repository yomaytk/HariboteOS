[bits 32]

		global 	api_putchar
		global  api_end
		global 	api_putstr0

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

api_end:
		mov 	edx, 4
		int 	0x40