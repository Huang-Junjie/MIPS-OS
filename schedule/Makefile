include ../tools/include.mk

INCLUDE := -I../include/ -I../lib/ -I../trap/ -I../process/ -I../pmm/ -I../vmm/


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: sched.o sched_rr.o

clean:
	rm -rf *.o


