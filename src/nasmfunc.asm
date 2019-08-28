; naskfunc
; TAB=4

;[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
;[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q
;[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��
;[FILE "naskfunc.nas"]			; �\�[�X�t�@�C�������

section .text

		GLOBAL	io_hlt
		GLOBAL	write_mem8

io_hlt:	; void io_hlt(void);
		HLT
		RET

write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; [ESP+4]��addr�������Ă���̂ł����ECX�ɓǂݍ���
		MOV		AL,[ESP+8]		; [ESP+8]��data�������Ă���̂ł����AL�ɓǂݍ���
		MOV		[ECX],AL
		RET
