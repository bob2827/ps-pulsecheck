
CFLAGS=--std=c99  -D_POSIX_SOURCE -D_BSD_SOURCE
all: p1 p2 p3

p1: p1.c
	$(CC) $(CFLAGS) p1.c -g -o p1

p2: p2.c pulsecheck.h
	$(CC) $(CFLAGS) p2.c -g -o p2
