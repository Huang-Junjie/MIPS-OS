include ../tools/include.mk

INCLUDE := -I../include/lib/ -I../drivers/gxconsole/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: printf.o string.o

clean:
	rm -rf *.o


