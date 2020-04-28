include ../tools/include.mk

INCLUDE := -I./ -Ilib/ -I../include


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: user_code.o lib/umain.o lib/ulib.o lib/syscall.o lib/syscall_asm.o lib/printf.o

clean:
	rm -rf *.o
	rm -rf lib/*.o

