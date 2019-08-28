; ; haribote-ipl
; ; TAB=4

; CYLS	EQU		10				; �ǂ��܂œǂݍ��ނ�

; 		ORG		0x7c00			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; ; �ȉ��͕W���I��FAT12�t�H�[�}�b�g�t���b�s�[�f�B�X�N�̂��߂̋L�q

; 		JMP		entry
; 		DB		0x90
; 		DB		"HARIBOTE"		; �u�[�g�Z�N�^�̖��O�����R�ɏ����Ă悢�i8�o�C�g�j
; 		DW		512				; 1�Z�N�^�̑傫���i512�ɂ��Ȃ���΂����Ȃ��j
; 		DB		1				; �N���X�^�̑傫���i1�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
; 		DW		1				; FAT���ǂ�����n�܂邩�i���ʂ�1�Z�N�^�ڂ���ɂ���j
; 		DB		2				; FAT�̌��i2�ɂ��Ȃ���΂����Ȃ��j
; 		DW		224				; ���[�g�f�B���N�g���̈�̑傫���i���ʂ�224�G���g���ɂ���j
; 		DW		2880			; ���̃h���C�u�̑傫���i2880�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
; 		DB		0xf0			; ���f�B�A�̃^�C�v�i0xf0�ɂ��Ȃ���΂����Ȃ��j
; 		DW		9				; FAT�̈�̒����i9�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
; 		DW		18				; 1�g���b�N�ɂ����̃Z�N�^�����邩�i18�ɂ��Ȃ���΂����Ȃ��j
; 		DW		2				; �w�b�h�̐��i2�ɂ��Ȃ���΂����Ȃ��j
; 		DD		0				; �p�[�e�B�V�������g���ĂȂ��̂ł����͕K��0
; 		DD		2880			; ���̃h���C�u�傫����������x����
; 		DB		0,0,0x29		; �悭�킩��Ȃ����ǂ��̒l�ɂ��Ă����Ƃ����炵��
; 		DD		0xffffffff		; ���Ԃ�{�����[���V���A���ԍ�
; 		DB		"HARIBOTEOS "	; �f�B�X�N�̖��O�i11�o�C�g�j
; 		DB		"FAT12   "		; �t�H�[�}�b�g�̖��O�i8�o�C�g�j
; 		RESB	18				; �Ƃ肠����18�o�C�g�����Ă���

; ; �v���O�����{��

; entry:
; 		MOV		AX,0			; ���W�X�^������
; 		MOV		SS,AX
; 		MOV		SP,0x7c00
; 		MOV		DS,AX

; ; �f�B�X�N��ǂ�

; 		MOV		AX,0x0820
; 		MOV		ES,AX
; 		MOV		CH,0			; �V�����_0
; 		MOV		DH,0			; �w�b�h0
; 		MOV		CL,2			; �Z�N�^2
; readloop:
; 		MOV		SI,0			; ���s�񐔂𐔂��郌�W�X�^
; retry:
; 		MOV		AH,0x02			; AH=0x02 : �f�B�X�N�ǂݍ���
; 		MOV		AL,1			; 1�Z�N�^
; 		MOV		BX,0
; 		MOV		DL,0x00			; A�h���C�u
; 		INT		0x13			; �f�B�X�NBIOS�Ăяo��
; 		JNC		next			; �G���[�������Ȃ����next��
; 		ADD		SI,1			; SI��1�𑫂�
; 		CMP		SI,5			; SI��5���r
; 		JAE		error			; SI >= 5 ��������error��
; 		MOV		AH,0x00
; 		MOV		DL,0x00			; A�h���C�u
; 		INT		0x13			; �h���C�u�̃��Z�b�g
; 		JMP		retry
; next:
; 		MOV		AX,ES			; �A�h���X��0x200�i�߂�
; 		ADD		AX,0x0020
; 		MOV		ES,AX			; ADD ES,0x020 �Ƃ������߂��Ȃ��̂ł������Ă���
; 		ADD		CL,1			; CL��1�𑫂�
; 		CMP		CL,18			; CL��18���r
; 		JBE		readloop		; CL <= 18 ��������readloop��
; 		MOV		CL,1
; 		ADD		DH,1
; 		CMP		DH,2
; 		JB		readloop		; DH < 2 ��������readloop��
; 		MOV		DH,0
; 		ADD		CH,1
; 		CMP		CH,CYLS
; 		JB		readloop		; CH < CYLS ��������readloop��

; ; �ǂݏI������̂�haribote.sys�����s���I

; 		MOV		[0x0ff0],CH		; IPL���ǂ��܂œǂ񂾂̂�������
; 		JMP		0xc200

; error:
; 		MOV		SI,msg
; putloop:
; 		MOV		AL,[SI]
; 		ADD		SI,1			; SI��1�𑫂�
; 		CMP		AL,0
; 		JE		fin
; 		MOV		AH,0x0e			; �ꕶ���\���t�@���N�V����
; 		MOV		BX,15			; �J���[�R�[�h
; 		INT		0x10			; �r�f�IBIOS�Ăяo��
; 		JMP		putloop
; fin:
; 		HLT						; ��������܂�CPU���~������
; 		JMP		fin				; �������[�v
; msg:
; 		DB		0x0a, 0x0a		; ���s��2��
; 		DB		"load error"
; 		DB		0x0a			; ���s
; 		DB		0

; 		RESB	0x7dfe-($-$$)		; 0x7dfe�܂ł�0x00�Ŗ��߂閽��

; 		DB		0x55, 0xaa
; haribote-ipl
; TAB=4

CYLS    EQU     10              ; �ǂ��܂œǂݍ��ނ� (CYLinderS)

        ORG     0x7c00          ; ���̃v���O��������������̂ǂ��ɓǂݍ��܂�邩

; discription for floppy disk
        JMP     entry           ; BS_JmpBoot
        DB      0x90            ; BS_JmpBoot
        DB      "HARIBOTE"      ; BS_OEMName    8B
        DW      512             ; BPB_BytsPerSec
        DB      1               ; BPB_SecPerClu
        DW      1               ; BPB_RevdSecCnt    : ����BPB���܂ރu�[�g�Z�N�^�̂�
        DB      2               ; BPB_NumFATs       : FAT�̌� (���̃t�B�[���h�̒l�͏��2�ɐݒ肷�ׂ��ł���)
        DW      224             ; BPB_RootEntCnt
        DW      2880            ; BPB_TotSec16
        DB      0xf0            ; BPB_Media
        DW      9               ; BPB_FATSz16
        DW      18              ; BPB_SecPerTrk
        DW      2               ; BPB_NumHeads
        DD      0               ; BPB_HiddSec
        DD      2880            ; BPB_TotSec32

        ; FAT12/16�ɂ�����I�t�Z�b�g36�ȍ~�̃t�B�[���h
        DB      0x00            ; BS_DrvNum
        DB      0x00            ; BS_Reserved1
        DB      0x29            ; BS_BootSig

        DD      0xffffffff      ; BS_VolID
        DB      "HARIBOTEOS "   ; BS_VolLab     11B
        DB      "FAT12   "      ; BS_FilSysType 8B
        RESB    18              ; �Ƃ肠����18�o�C�g�J���Ă���


; START BS_BootCode 64(0x14)   448(0x1C0)
entry:
        MOV     AX, 0           ; initialize Accumulator(resister)
        MOV     SS, AX          ; Stack Segment
        MOV     SP, 0x7c00      ; Stack Pointer
        MOV     DS, AX          ; Data Segment      : �Ԓn�w��̂Ƃ��d�v


; load disk
        MOV     AX, 0x0820
        MOV     ES, AX          ; buffer address       0x0820
        MOV     CH, 0           ; cylinder  0
        MOV     DH, 0           ; head      0
        MOV     CL, 2           ; sector    2

readloop:
        MOV     SI, 0           ; ���s�񐔂𐔂��郌�W�X�^

retry:
        MOV     AH, 0x02        ; acumulator high   : 0x02 - read disk
        MOV     AL, 1           ; acumulator low    : sector    1
        MOV     BX, 0           ; buffer address    0x0000
                                ; ES:BX, ES�͑���ς�
        MOV     DL, 0x00        ; data low          : drive number
        INT     0x13            ; BIOS call
        JNC     next            ; jump if not carry

        ADD     SI, 1           ; increment SI
        CMP     SI, 5
        JAE     error           ; SI >= 5 then jump to error

        MOV     AH, 0x00        ; 0x00 - reset
        MOV     DL, 0x00        ; A drive
        INT     0x13            ; reset drive
        JMP     retry

next:
        ; add 0x20 to ES
        ; �����BX��512�𑫂��Ă��悢
        MOV     AX, ES          ; 0x20�����A�h���X��i�߂�
        ADD     AX, 0x0020      ; 512 / 16 = 0x20
        MOV     ES, AX

        ; increment CL (sector number)
        ADD     CL, 1
        CMP     CL, 18
        JBE     readloop

        ; �f�B�X�N�̃E����
        MOV     CL, 1           ; reset sector
        ADD     DH, 1           ; reverse HEAD
        CMP     DH, 2
        JB      readloop

        ; next Cylinder
        mov     DH, 0           ; reset HEAd
        ADD     CH, 1           ; cylinder += 1
        CMP     CH, CYLS
        JB      readloop

; �u�[�g�Z�N�^�̓ǂݍ��݂��I������̂�OS�{�̂����s
		MOV		[0x0ff0],CH		; IPL���ǂ��܂œǂ񂾂̂�������
        JMP     0xc200

error:
        MOV     SI, msg

putloop:
        MOV     AL, [SI]        ; BYTE (accumulator low)
        ADD     SI, 1           ; increment
        CMP     AL, 0           ; �I������
        JE      fin             ; jump to fin if equal to 0

        MOV     AH, 0x0e        ; 1 char-function
        MOV     BX, 15          ; color code
        INT     0x10            ; interrupt, call BIOS
        JMP     putloop

fin:
		HLT						; ��������܂�CPU���~������
		JMP		fin				; �������[�v

msg:
        DB      0x0a, 0x0a
        DB      "load error"
        DB      0x0a
        DB      0               ; end point

        RESB    0x7dfe - 0x7c00 - ($ - $$)  ; ���݂̏ꏊ����0x1fd�܂�(�c��̖��g�p�̈�)��0�Ŗ��߂�B
                                            ; 0x7c00�X�^�[�g�Ȃ̂ł��̕��������Ă���
; END BS_BootCode
        DB      0x55, 0xaa      ; BS_BootSign, boot signature