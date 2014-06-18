CFLAGS=--std=c99  -D_POSIX_SOURCE -D_BSD_SOURCE

#Comment out to remove extra logging vomit
CFLAGS+=-DDBGINFO

all: p1 p2 p3

p1: pulsecheck.c p1.c
	$(CC) $(CFLAGS) $^ -g -o p1

p2: pulsecheck.c p2.c
	$(CC) $(CFLAGS) $^ -g -o p2

p3: pulsecheck.c p3.c
	$(CC) $(CFLAGS) $^ -g -o p3

clean:
	@-$(RM) -rf p1 p2 p3
