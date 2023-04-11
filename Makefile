CFLAGS=-std=c11 -g -static -Wall -fcommon
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

seacc: $(OBJS)
	$(CC) -o seacc $(OBJS) $(LDFLAGS)

$(OBJS): seacc.h

test: seacc
	test/test.sh

clean:
	rm -f seacc *.o *.swp *.swo tmp*

.PHONY: test clean
