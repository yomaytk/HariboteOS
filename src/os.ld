OUTPUT_FORMAT("binary");

SECTIONS
{
    .head 0x0 : {
        LONG(64 * 1024)       /*  0x0000 : stack+.data+heap の大きさ（4KBの倍数） */
        LONG(0x6e69616d)      /*  0x0004 : シグネチャ "main" */
        LONG(0)               /*  0x0008 : mmarea の大きさ（4KBの倍数） */
        LONG(0x310000)        /*  0x000c : スタック初期値＆.data転送先 */
        LONG(SIZEOF(.data))   /*  0x0010 : .dataサイズ */
        LONG(LOADADDR(.data)) /*  0x0014 : .dataの初期値列のファイル位置	 */
        LONG(0xE9000000)      /*  0x0018 : 0xE9000000 */
        LONG(main - 0x20)     /*  0x001c : エントリアドレス - 0x20 */
        LONG(0)               /*  0x0020 : heap領域（malloc領域）開始アドレス */
    }

    .text : { *(.text) }

    .data 0x310000 : AT ( ADDR(.text) + SIZEOF(.text) ) {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

    /DISCARD/ : { *(.eh_frame) }

}