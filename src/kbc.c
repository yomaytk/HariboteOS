#include<stdio.h>
#include"bootpack.h"

void wait_KBC_sendready()
{
	/* �L�[�{�[�h�R���g���[�����f�[�^���M�\�ɂȂ�̂�҂� */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard()
{
	/* �L�[�{�[�h�R���g���[���̏����� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

struct FIFO8 keyfifo;

void inthandler21(int *esp)
/* PS/2�L�[�{�[�h����̊��荞�� */
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	// PIC�Ɋ��荞�݂̎󗝂�ʒm
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);

	return;
}
