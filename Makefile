CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

seacc: $(OBJS)
	$(CC) -o seacc $(OBJS) $(LDFLAGS)

$(OBJS): seacc.h

test: seacc
	./test.sh

clean:
	rm -f seacc *.o *.swp *.swo tmp*

.PHONY: test clean