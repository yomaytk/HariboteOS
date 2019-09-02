/*===== mouse process =====*/

#include<stdio.h>
#include"bootpack.h"

void enable_mouse(struct MOUSE_DEC *mdec)
{
	/* �}�E�X�L�� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return; /* ���܂�������ACK(0xfa)�����M����Ă��� */
}
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* �}�E�X��0xfa��҂��Ă���i�K */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* �}�E�X��1�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[0] = dat;
		mdec->phase = 2;
		return 0;
	}
	if (mdec->phase == 2) {
		/* �}�E�X��2�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* �}�E�X��3�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		return 1;
	}
	return -1; /* �����ɗ��邱�Ƃ͂Ȃ��͂� */
}