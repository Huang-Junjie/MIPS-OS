include ../../tools/include.mk

INCLUDE := -I../../include/

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

.PHONY: all clean

all: clock.o

clean:
	rm -rf *.o


