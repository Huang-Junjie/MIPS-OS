/* gxemul 提供的console device
 * http://gavare.se/gxemul/gxemul-stable/doc/experiments.html#expdevices
 */
#define DEV_CONS_ADDRESS 0x10000000
#define DEV_CONS_PUTGETCHAR 0x00


/* 使用MIPS非映射非缓存的kseg1段内核虚拟地址 */
#define PHYSADDR_OFFSET 0xA0000000


#define PUTCHAR_ADDRESS (PHYSADDR_OFFSET + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR)


void putChar(char ch) {
    *((volatile unsigned char *) PUTCHAR_ADDRESS) = ch;
}
