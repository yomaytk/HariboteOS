[BITS 32]

hello:
		mov 	al,'h'
		int		0x40
		mov 	al,'e'
		int		0x40		
		mov 	al,'l'
		int		0x40		
		mov 	al,'l'
		int		0x40
		mov 	al,'o'
		int		0x40		
		retf