
[bits 32]						; make machine language for 32bit 

		global	io_hlt, io_cli, io_sti, io_stihlt
		global	io_in8,  io_in16,  io_in32
		global	io_out8, io_out16, io_out32
		global	io_load_eflags, io_store_eflags
		global	load_gdtr, load_idtr
		global	asm_inthandler21, asm_inthandler27, asm_inthandler2c
		global  asm_inthandler20
		global	load_cr0, store_cr0
		global  load_tr
		global	farjmp, farcall
		global  start_app
		global	memtest_sub, mts_loop, mts_fin
		extern	inthandler21, inthandler27, inthandler2c, inthandler20
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
		PUSHFD		; PUSH EFLAGS という意味
		POP		EAX
		RET

io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS という意味
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
		mov		ax, ss
		cmp 	ax, 1*8
		jne 	.from_app
; if segment is equal to 1*8, interrupt yield when application is execting
		mov		eax, esp
		push  	ss
		push 	eax
		MOV		AX,SS
		mov		ds, ax
		mov		es, ax
		call	inthandler21
		add 	esp, 8
		popad
		pop		ds
		pop		es
		iretd
.from_app
		mov 	eax, 1*8
		mov 	ds, ax
		mov 	ecx, [0xfe4]	; esp for OS system
		add 	ecx, -8
		mov 	[ecx+4], ss
		mov 	[ecx], esp
		mov 	ss, ax
		mov 	es, ax
		mov 	esp, ecx
		call 	inthandler21
		pop 	ecx		; pop esp for application
		pop		eax		; pop ss for application
		mov 	esp, ecx
		mov 	ss, ax
		popad
		pop 	ds
		pop 	es
		iretd


asm_inthandler20:
		push	es
		push	ds
		pushad
		mov		ax, ss
		cmp 	ax, 1*8
		jne 	.from_app
; if segment is equal to 1*8, interrupt yield when application is execting
		mov		eax, esp
		push  	ss
		push 	eax
		MOV		AX,SS
		mov		ds, ax
		mov		es, ax
		call	inthandler20
		add 	esp, 8
		popad
		pop		ds
		pop		es
		iretd
.from_app
		mov 	eax, 1*8
		mov 	ds, ax
		mov 	ecx, [0xfe4]	; esp for OS system
		add 	ecx, -8
		mov 	[ecx+4], ss
		mov 	[ecx], esp
		mov 	ss, ax
		mov 	es, ax
		mov 	esp, ecx
		call 	inthandler20
		pop 	ecx		; pop esp for application
		pop		eax		; pop ss for application
		mov 	esp, ecx
		mov 	ss, ax
		popad
		pop 	ds
		pop 	es
		iretd


asm_inthandler27:
		push	es
		push	ds
		pushad
		mov		ax, ss
		cmp 	ax, 1*8
		jne 	.from_app
; if segment is equal to 1*8, interrupt yield when application is execting
		mov		eax, esp
		push  	ss
		push 	eax
		MOV		AX,SS
		mov		ds, ax
		mov		es, ax
		call	inthandler27
		add 	esp, 8
		popad
		pop		ds
		pop		es
		iretd
.from_app
		mov 	eax, 1*8
		mov 	ds, ax
		mov 	ecx, [0xfe4]	; esp for OS system
		add 	ecx, -8
		mov 	[ecx+4], ss
		mov 	[ecx], esp
		mov 	ss, ax
		mov 	es, ax
		mov 	esp, ecx
		call 	inthandler27
		pop 	ecx		; pop esp for application
		pop		eax		; pop ss for application
		mov 	esp, ecx
		mov 	ss, ax
		popad
		pop 	ds
		pop 	es
		iretd


asm_inthandler2c:
		push	es
		push	ds
		pushad
		mov		ax, ss
		cmp 	ax, 1*8
		jne 	.from_app
; if segment is equal to 1*8, interrupt yield when application is execting
		mov		eax, esp
		push  	ss
		push 	eax
		MOV		AX,SS
		mov		ds, ax
		mov		es, ax
		call	inthandler2c
		add 	esp, 8
		popad
		pop		ds
		pop		es
		iretd
.from_app
		mov 	eax, 1*8
		mov 	ds, ax
		mov 	ecx, [0xfe4]	; esp for OS system
		add 	ecx, -8
		mov 	[ecx+4], ss
		mov 	[ecx], esp
		mov 	ss, ax
		mov 	es, ax
		mov 	esp, ecx
		call 	inthandler2c
		pop 	ecx		; pop esp for application
		pop		eax		; pop ss for application
		mov 	esp, ecx
		mov 	ss, ax
		popad
		pop 	ds
		pop 	es
		iretd



memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; （EBX, ESI, EDI も使いたいので）
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

; asm_os_api:								; push register to stack and call os_api
; 		pushad		;store register
; 		pushad		;use on os_api
; 		call	os_api
; 		add 	esp, 32
; 		popad
; 		iretd

asm_os_api:
		; 都合のいいことに最初から割り込み禁止になっている
		PUSH	DS
		PUSH	ES
		PUSHAD		; 保存のためのPUSH
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-40
		MOV		[ECX+32],ESP	; アプリのESPを保存
		MOV		[ECX+36],SS		; アプリのSSを保存

; PUSHADした値をシステムのスタックにコピーする
		MOV		EDX,[ESP   ]
		MOV		EBX,[ESP+ 4]
		MOV		[ECX   ],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+ 4],EBX	; hrb_apiに渡すためコピー
		MOV		EDX,[ESP+ 8]
		MOV		EBX,[ESP+12]
		MOV		[ECX+ 8],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+12],EBX	; hrb_apiに渡すためコピー
		MOV		EDX,[ESP+16]
		MOV		EBX,[ESP+20]
		MOV		[ECX+16],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+20],EBX	; hrb_apiに渡すためコピー
		MOV		EDX,[ESP+24]
		MOV		EBX,[ESP+28]
		MOV		[ECX+24],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+28],EBX	; hrb_apiに渡すためコピー

		MOV		ES,AX			; 残りのセグメントレジスタもOS用にする
		MOV		SS,AX
		MOV		ESP,ECX
		STI			; やっと割り込み許可

		CALL	os_api

		MOV		ECX,[ESP+32]	; アプリのESPを思い出す
		MOV		EAX,[ESP+36]	; アプリのSSを思い出す
		CLI
		MOV		SS,AX
		MOV		ESP,ECX
		POPAD
		POP		ES
		POP		DS
		IRETD		; この命令が自動でSTIしてくれる


start_app:		; void start_app(int eip, int cs, int esp, int ds);
		pushad		; 32ビットレジスタを全部保存しておく
		mov		eax,[esp+36]	; アプリ用のEIP
		mov		ecx,[esp+40]	; アプリ用のCS
		mov		edx,[esp+44]	; アプリ用のESP
		mov		ebx,[esp+48]	; アプリ用のDS/SS
		mov		[0xfe4],esp		; OS用のESP
		cli			; 切り替え中に割り込みが起きてほしくないので禁止
		mov		es,bx
		mov		ss,bx
		mov		ds,bx
		mov		fs,bx
		mov		gs,bx
		mov		esp,edx
		sti			; 切り替え完了なので割り込み可能に戻す
		push	ecx				; far-CALLのためにPUSH（cs）
		push	eax				; far-CALLのためにPUSH（eip）
		call	far [esp]		; アプリを呼び出す

;	アプリが終了するとここに帰ってくる

		mov		eax,1*8			; OS用のDS/SS
		cli			; また切り替えるので割り込み禁止
		mov		es,ax
		mov		ss,ax
		mov		ds,ax
		mov		fs,ax
		mov		gs,ax
		mov		esp,[0xfe4]
		sti		; 切り替え完了なので割り込み可能に戻す
		popad	; 保存しておいたレジスタを回復
		ret
