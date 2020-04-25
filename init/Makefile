include ../tools/include.mk

INCLUDE := -I../include -I../lib/ -I../trap/ -I../pmm/ -I../vmm/  -I../drivers/gxclock/  -I../process/  -I../schedule/

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@
	

.PHONY: all clean

all: start.o init.o

clean:
	rm -rf *.o



