[BITS 32]

		mov 	ebx,msg
		mov 	edx,2
		int 	0x40
		mov 	edx, 4
		int 	0x40

msg:
		db "hello",0