CFLAGS=-std=c11 -g -static

seacc: seacc.c

test: seacc
	./test.sh

clean:
	rm -f seacc *.o *.swp *.swo tmp*

.PHONY: test clean
