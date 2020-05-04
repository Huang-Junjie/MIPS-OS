include ../tools/include.mk

INCLUDE := -I../include/ -I../lib/ -I../trap/ -I../process/ -I../pmm/ -I../vmm/ -I../schedule/ -I../sync/ 


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: proc.o proc_asm.o entry.o

clean:
	rm -rf *.o


