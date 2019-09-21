BOTPAK  EQU    0x00280000    ; BOOTPACK     | bootpack�̃��[�h��
DSKCAC  EQU    0x00100000    ; DISK CACHE   | �f�B�X�N�L���b�V���̏ꏊ
DSKCAC0 EQU    0x00008000    ; DISK CACHE 0 | �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO�֌W
; > �������Ă���ꏊ��0x0ff0�Ԓn���ӂł����A�������}�b�v�ɂ��Ƃ��̂ւ���N�ɂ��g���Ă��Ȃ��悤�ł���
CYLS    EQU     0x0ff0      ; �u�[�g�Z�N�^���ݒ肷��
LEDS    EQU     0x0ff1      ; LED STATE
VMODE   EQU     0x0ff2      ; VIDEO MODE | �F���Ɋւ�����i���r�b�g�J���[���j
SCRNX   EQU     0x0ff4      ; SCREEN X   | �𑜓xX
SCRNY   EQU     0x0ff6      ; SCREEN Y   | �𑜓xY
VRAM    EQU     0x0ff8      ; VIDEO RAM  | �O���t�B�b�N�o�b�t�@�̊J�n�Ԓn

        ;=======================================================================
        ORG     0xc200      ; 0xc200 = 0x8000 + 0x4200
                            ; �C���[�W�t�@�C���� 0x4200 �A�h���X�Ԗڂɏ������܂�Ă���
                            ; �܂�,��� 0x8000 �ȍ~���g�����ƂɌ��߂Ă���

        ;=======================================================================
        ; [INT(0x10); �r�f�I�֌W](http://oswiki.osask.jp/?%28AT%29BIOS#n5884802)
        ; �r�f�I���[�h�ݒ�
        ;   AH = 0x00;
        ;   AL = ���[�h�F (�}�C�i�[�ȉ�ʃ��[�h�͏ȗ����Ă��܂�)
        ;     0x03�F16�F�e�L�X�g�A80x25
        ;     0x12�FVGA�O���t�B�b�N�X�A640x480x4bit�J���[�A�Ǝ��v���[���A�N�Z�X
        ;     0x13�FVGA�O���t�B�b�N�X�A320x200x8bit�J���[�A�p�b�N�h�s�N�Z��
        ;     0x6a�F�g��VGA�O���t�B�b�N�X�A800x600x4bit�J���[�A�Ǝ��v���[���A�N�Z�X�i�r�f�I�J�[�h�ɂ���Ă̓T�|�[�g����Ȃ��j
        ;   �߂�l�F�Ȃ�
VBEMODE	EQU		0x105			; 1024 x  768 x 8bit�J���[
; �i��ʃ��[�h�ꗗ�j
;	0x100 :  640 x  400 x 8bit�J���[
;	0x101 :  640 x  480 x 8bit�J���[
;	0x103 :  800 x  600 x 8bit�J���[
;	0x105 : 1024 x  768 x 8bit�J���[
;	0x107 : 1280 x 1024 x 8bit�J���[

; VBE���݊m�F
		JMP		scrn320
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; VBE�̃o�[�W�����`�F�b�N

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

; ��ʃ��[�h���𓾂�

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ��ʃ��[�h���̊m�F

		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]
		AND		AX,0x0080
		JZ		scrn320			; ���[�h������bit7��0�������̂ł�����߂�

; ��ʃ��[�h�̐؂�ւ�

		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13			; VGA�O���t�B�b�N�X�A320x200x8bit�J���[
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
		JMP		keystatus

; �L�[�{�[�h��LED��Ԃ�BIOS�ɋ����Ă��炤

keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

		; MOV		BX,0x4101		; VBE��640x480x8bit�J���[
		; MOV		AX,0x4f02
        ; MOV     AL, 0x12    ; VGA graphics, 320x200x(8 bit color)
        ; MOV     AH, 0x00
        ; INT     0x10

        ;=======================================================================
        ; ��ʃ��[�h����������
        ; MOV     BYTE [VMODE], 8           ; Video MODE
        ; MOV     WORD [SCRNX], 640        ; SCReeN X
        ; MOV     WORD [SCRNY], 480         ; SCReeN Y
        ; MOV     DWORD [VRAM], 0x000a0000  ; Video RAM
										; > VRAM��0xa0000�`0xaffff��64KB�ł��B�����Ɍ����ƁA320x200=64000�Ȃ̂ŁA62.5KB�ł���.
										;
										; > [VRAM]�� 0xa0000 �����Ă���̂ł����APC �̐��E�� VRAM �Ƃ����̂̓r�f�I�����̂��Ƃ�
										; > �uvideo RAM�v�Ə����A��ʗp�̃������̂��Ƃł��B���̃������́A�������f�[�^���L�����邱�Ƃ���
										; > ���ʂ�ł��܂��B������VRAM�͕��ʂ̃������ȏ�̑��݂ŁA���ꂼ��̔Ԓn����ʏ�̉�f�ɑΉ�
										; > ���Ă��āA����𗘗p���邱�Ƃŉ�ʂɊG���o�����Ƃ��ł���̂ł��B

        ;=======================================================================
        ; [INT(0x16); �L�[�{�[�h�֌W - (AT)BIOS - os-wiki](http://oswiki.osask.jp/?%28AT%29BIOS#lb9f3e72)
        ; �L�[���b�N���V�t�g��Ԏ擾
        ;   AH = 0x02;
        ;   �߂�l�F
        ;   AL == ��ԃR�[�h�F
        ;     bit0�F�E�V�t�g
        ;     bit1�F���V�t�g
        ;     bit2�FCtrl
        ;     bit3�FAlt
        ;     bit4�FScroll���b�N
        ;     bit5�FNum���b�N
        ;     bit6�FCaps���b�N
        ;     bit7�FInsert���[�h
        ; BIOS (16 bit mode) ��������擾
        ; MOV     AH, 0x02    ; �L�[���b�N���V�t�g��Ԏ擾
        ; INT     0x16        ; Keyboard BIOS
        ; MOV     [LEDS], AL  ; LED State

        ; PIC����؂̊��荞�݂��󂯕t���Ȃ��悤�ɂ���
        ; AT�݊��@�̎d�l�ł́APIC�̏�����������Ȃ�A
        ; ������CLI�O�ɂ���Ă����Ȃ��ƁA���܂Ƀn���O�A�b�v����
        ; PIC�̏������͂��Ƃł��

        MOV     AL, 0xff
        OUT     0x21, AL
        NOP                   ; OUT���߂�A��������Ƃ��܂������Ȃ��@�킪����炵���̂�
        OUT     0xa1, AL

        CLI                   ; �����CPU���x���ł����荞�݋֎~

        ; CPU����1MB�ȏ�̃������ɃA�N�Z�X�ł���悤�ɁAA20GATE��ݒ�

        CALL waitkbdout
        MOV  AL,0xd1
        OUT  0x64,AL
        CALL waitkbdout
        MOV  AL,0xdf          ; enable A20
        OUT  0x60,AL
        CALL waitkbdout

        ; �v���e�N�g���[�h�ڍs
        
        ;[INSTRSET "i486p"]    ; i486�̖��߂܂Ŏg�������Ƃ����L�q
        ; �����Ŏw�肷��̂ł͂Ȃ�gcc��compile����ۂ�i486�Ŏw��

        LGDT [GDTR0]   ; �b��GDT��ݒ�
        MOV  EAX,CR0
        AND  EAX,0x7fffffff ; bit31��0�ɂ���i�y�[�W���O�֎~�̂��߁j
        OR  EAX,0x00000001 ; bit0��1�ɂ���i�v���e�N�g���[�h�ڍs�̂��߁j
        MOV  CR0,EAX
        JMP  pipelineflush
pipelineflush:
        MOV  AX,1*8   ;  �ǂݏ����\�Z�O�����g32bit
        MOV  DS,AX
        MOV  ES,AX
        MOV  FS,AX
        MOV  GS,AX
        MOV  SS,AX

        ; bootpack�̓]��

        MOV  ESI,bootpack ; �]����
        MOV  EDI,BOTPAK  ; �]����
        MOV  ECX,512*1024/4
        ;MOV  ECX, 131072
        CALL memcpy

        ; ���łɃf�B�X�N�f�[�^���{���̈ʒu�֓]��

        ; �܂��̓u�[�g�Z�N�^����

        MOV  ESI,0x7c00  ; �]����
        MOV  EDI,DSKCAC  ; �]����
        MOV  ECX,512/4
        ;MOV  ECX, 128
        CALL memcpy

        ; �c��S��

        MOV  ESI,DSKCAC0+512 ; �]����
        MOV  EDI,DSKCAC+512 ; �]����
        MOV  ECX,0
        MOV  CL,BYTE [CYLS]
        IMUL ECX,512*18*2/4 ; �V�����_������o�C�g��/4�ɕϊ�
        ;IMUL ECX, 4608
        SUB  ECX,512/4  ; IPL�̕�������������
        ;SUB  ECX, 128  ; IPL�̕�������������
        CALL memcpy

        ; asmhead�ł��Ȃ���΂����Ȃ����Ƃ͑S�����I������̂ŁA
        ; ���Ƃ�bootpack�ɔC����
        
        ; bootpack�̋N��

        MOV  EBX,BOTPAK
        MOV  ECX,[EBX+16]
        ADD  ECX,3   ; ECX += 3;
        SHR  ECX,2   ; ECX /= 4;
        JZ  skip   ; �]������ׂ����̂��Ȃ�
        MOV  ESI,[EBX+20] ; �]����
        ADD  ESI,EBX
        MOV  EDI,[EBX+12] ; �]����
        CALL memcpy
skip:
        MOV  ESP,[EBX+12] ; �X�^�b�N�����l
        JMP  DWORD 2*8:0x0000001b

waitkbdout:
        IN   AL,0x64
        AND   AL,0x02
		;IN	 AL,0x60			; <- remove����
        JNZ  waitkbdout  ; AND�̌��ʂ�0�łȂ����waitkbdout��
        RET

memcpy:
        MOV  EAX,[ESI]
        ADD  ESI,4
        MOV  [EDI],EAX
        ADD  EDI,4
        SUB  ECX,1
        JNZ  memcpy   ; �����Z�������ʂ�0�łȂ����memcpy��
        RET
        ; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

        ALIGNB 16
GDT0:
        RESB 8    ; �k���Z���N�^
        DW  0xffff,0x0000,0x9200,0x00cf ; �ǂݏ����\�Z�O�����g32bit
        DW  0xffff,0x0000,0x9a28,0x0047 ; ���s�\�Z�O�����g32bit�ibootpack�p�j

        DW  0
GDTR0:
        DW  8*3-1
        DD  GDT0

        ALIGNB 16
bootpack: