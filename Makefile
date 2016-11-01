CC?=clang
CFLAGS?=-Wall -Wextra -pedantic -std=c11
LDFLAGS=-lgsl -lgslcblas -lm

default: fairdice.debug fairdice

fairdice.debug: fairdice.c
	$(CC) $(CFLAGS) $(LDFLAGS) -g -o $@ $<

fairdice: fairdice.c
	$(CC) $(CFLAGS) $(LDFLAGS) -O2 -o $@ $<

fairdice.prof: fairdice.c
	$(CC) $(CFLAGS) $(LDFLAGS) -g -lprofiler -o $@ $<

.PHONY: default
