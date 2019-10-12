
[bits 32]						; make machine language for 32bit 

		global	io_hlt, io_cli, io_sti, io_stihlt
		global	io_in8,  io_in16,  io_in32
		global	io_out8, io_out16, io_out32
		global	io_load_eflags, io_store_eflags
		global	load_gdtr, load_idtr
		global	asm_inthandler21, asm_inthandler27, asm_inthandler2c
		global  asm_inthandler20
		global 	asm_inthandler0d
		global	load_cr0, store_cr0
		global  load_tr
		global	farjmp, farcall
		global  start_app
		global	memtest_sub, mts_loop, mts_fin
		extern	inthandler21, inthandler27, inthandler2c, inthandler20
		extern 	inthandler0d
		global	asm_cons_putchar
		extern 	cons_putchar
		global	asm_os_api
		extern  os_api


section .text

io_hlt:	; void io_hlt(void);
		HLT
		RET

io_cli:	; void io_cli(void);
		CLI
		RET

io_sti:	; void io_sti(void);
		STI
		RET

io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS Ç∆Ç¢Ç§à”ñ°
		POP		EAX
		RET

io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS Ç∆Ç¢Ç§à”ñ°
		RET

load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET

asm_inthandler21:
		push	es
		push	ds
		pushad
		mov 	eax, esp
		push 	eax
		mov		ax, ss
		mov		ds, ax
		mov		es, ax
		call	inthandler21
		pop 	eax
		popad
		pop		ds
		pop		es
		iretd


asm_inthandler20:
		push	es
		push	ds
		pushad
		mov 	eax, esp
		push 	eax
		mov		ax, ss
		mov		ds, ax
		mov		es, ax
		call	inthandler20
		pop 	eax
		popad
		pop		ds
		pop		es
		iretd


asm_inthandler27:
		push	es
		push	ds
		pushad
		mov 	eax, esp
		push 	eax
		mov		ax, ss
		mov		ds, ax
		mov		es, ax
		call	inthandler27
		pop 	eax
		popad
		pop		ds
		pop		es
		iretd


asm_inthandler2c:
		push	es
		push	ds
		pushad
		mov 	eax, esp
		push 	eax
		mov		ax, ss
		mov		ds, ax
		mov		es, ax
		call	inthandler2c
		pop 	eax
		popad
		pop		ds
		pop		es
		iretd

asm_inthandler0d:
		push	es
		push	ds
		pushad
		mov 	eax, esp
		push 	eax
		mov		ax, ss
		mov		ds, ax
		mov		es, ax
		call	inthandler0d
		cmp 	eax, 0
		jne 	end_app
		pop 	eax
		popad
		pop		ds
		pop		es
		add 	esp, 4
		iretd

memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; ÅiEBX, ESI, EDI Ç‡égÇ¢ÇΩÇ¢ÇÃÇ≈Åj
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;

mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET

mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

farjmp:		; void farjmp(int eip, int cs)
		JMP		FAR	[ESP+4]				; eip, cs
		RET

farcall:	; void farcall(int eip, int cs)
		call 	far[esp+4]
		ret

asm_cons_putchar:
		sti
		pushad
		push	1
		; MOV		eax, 100
		and 	eax, 0xff
		push 	eax
		push 	DWORD [0x0fec]
		call 	cons_putchar
		add 	esp, 12
		popad
		iretd

asm_os_api:
		sti
		push	ds
		push	es
		pushad		; push for saving
		pushad 		; push for use at os_api
		mov 	ax, ss
		mov		es, ax
		mov		ds, ax
		call	os_api
		cmp 	eax, 0
		jne 	end_app
		add 	esp, 32
		popad
		pop		es
		pop		ds
		iretd		; sti automatically by this order
end_app:
		mov 	esp, [eax]
		pushad
		ret 	; back to app_exe

start_app:		; void start_app(int eip, int cs, int esp, int ds, int tss.esp0);
		pushad		; save all 32bit register
		mov		eax, [esp+36]	
		mov		ecx, [esp+40]	
		mov		edx, [esp+44]	
		mov		ebx, [esp+48]
		mov		ebp, [esp+52]
		mov 	[ebp], esp		; set esp and ss for OS system on [tss.esp0] and [tss.esp0+4]
		mov 	[ebp+4], ss
		mov		es, bx
		mov		ds, bx
		mov		fs, bx
		mov		gs, bx
		mov		esp, edx
; adjustment stack retf to application
		or 		ecx, 3	; set priviledge level 3 for segment of application ??? 
		or 		ecx, 3	; same above
		push 	ebx
		push 	edx
		push 	ecx
		push 	eax
		retf	; pop eax(eip for app) -> pop ecx(cs for app) -> jmp
