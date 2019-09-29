[BITS 32]

		mov 	ebx,msg
		mov 	edx,2
		int 	0x40
		retf

msg:
		db "hello",0