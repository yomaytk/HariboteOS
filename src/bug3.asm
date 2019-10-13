[bits 32]

		mov 	ebx, inf_msg

put_loop:
		mov 	edx, 2
		int 	0x40
		jmp 	put_loop

inf_msg:
		db 		"a", 0