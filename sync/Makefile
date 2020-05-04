include ../tools/include.mk

INCLUDE := -I../include/ -I../lib/ -I../trap/ -I../process/ -I../pmm/ -I../vmm/ -I../sync/


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: sync_asm.o wait.o sem.o check_sync.o

clean:
	rm -rf *.o


