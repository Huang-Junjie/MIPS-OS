include ../../tools/include.mk

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: all clean

all: console.o

clean:
	rm -rf *.o



