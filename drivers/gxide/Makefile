include ../../toos/include.mk

INCLUDE := -I../../include/ -I../../lib/
SWAPIMG := ../../gxemul/swap.img

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@



.PHONY: all clean $(SWAPIMG)

all: ide.o ide_asm.o $(SWAPIMG)

$(SWAPIMG):
	$(V)dd if=/dev/zero of=$@ bs=1024k count=128

clean:
	rm -rf *.o
	rm -rf $(SWAPIMG)


