include ../tools/include.mk

INCLUDE := -I../include/ -I../lib/ -I../trap/ -I../process/ -I../pmm/ -I../vmm/


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: pmm.o pmm_firstfit.o pmm_asm.o kmalloc.o

clean:
	rm -rf *.o


