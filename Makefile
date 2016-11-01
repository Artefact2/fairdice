CC?=clang
CFLAGS?=-Wall -Wextra -pedantic -std=c11

default: fairdice.debug fairdice

fairdice.debug: fairdice.c
	$(CC) $(CFLAGS) -g -o $@ $<

fairdice: fairdice.c
	$(CC) $(CFLAGS) -O2 -o $@ $<

fairdice.prof: fairdice.c
	$(CC) $(CFLAGS) -g -lprofiler -o $@ $<

.PHONY: default
