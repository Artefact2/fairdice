/* Author: Romain "Artefact2" Dal Maso <artefact2@gmail.com> */

/* This program is free software. It comes without any warranty, to the
 * extent permitted by applicable law. You can redistribute it and/or
 * modify it under the terms of the Do What The Fuck You Want To Public
 * License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#define warn(...) do {	  \
		char msg[256]; \
		snprintf(msg, 256, __VA_ARGS__); \
		fprintf(stderr, "%s(): %s\n", __func__, msg); \
	} while(0)

#define fatal(...) do {	  \
		warn(__VA_ARGS__); \
		exit(1); \
	} while(0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <gsl/gsl_sf_gamma.h>

#define MCTS (1 << 15)

void read_rolls(unsigned int, unsigned int* rcount, unsigned int* n);
unsigned int ecdf_distance(unsigned int, unsigned int*);
void ecdf_gen_mc_table(unsigned int sides, unsigned int n, unsigned int count, unsigned int*);
bool find_in_sorted_array(unsigned int val, unsigned int count, unsigned int* array, unsigned int* low, unsigned int* high);
double chisq_test(unsigned int sides, unsigned int n, unsigned int*);
void confidence_test(unsigned int sides, unsigned int n, unsigned int*);

int main(int argc, char** argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <num-sides> < rolls.txt\n", argv[0]);
		exit(2);
	}

	unsigned int sides = strtol(argv[1], NULL, 10);
	if(sides < 2) {
		fatal("must have at least 2 sides, got %d", sides);
	}

	unsigned int rcount[sides];
	unsigned int n;
	read_rolls(sides, rcount, &n);
	if(n % sides) {
		fatal("sample size (%d) not a multiple of sides (%d)", n, sides);
	}

	printf("SmpSize:   n=%d\n", n);

	unsigned int ecdf_table[MCTS];
	ecdf_gen_mc_table(sides, n, MCTS, ecdf_table);

	unsigned int dist = ecdf_distance(sides, rcount);
	unsigned int lo, hi;
	bool found;

	found = find_in_sorted_array(dist, MCTS, ecdf_table, &lo, &hi);
	printf("ECDF:      p%s%5.4f\n", found ? "=" : "<", found ? (1.0 - (double)(lo + hi) / (double)(2 * MCTS)) : ((double)(MCTS - lo) / (double)MCTS));

	printf("ChiSq:     p=%5.4f\n", chisq_test(sides, n, rcount));

	printf("ConfInt99:");
	confidence_test(sides, n, rcount);
	printf("\n");
}

void read_rolls(unsigned int sides, unsigned int* rcount, unsigned int* n) {
	char buf[16];
	unsigned int roll;

	memset(rcount, 0, sides * sizeof(unsigned int));
	*n = 0;

	while(!feof(stdin)) {
		if(fgets(buf, 16, stdin) == NULL) continue;
		if(buf[0] == '\n' || buf[0] == '\0') continue;
		
		roll = strtol(buf, NULL, 10);
		if(roll < 1 || roll > sides) {
			warn("ignoring roll %d", roll);
			continue;
		}

		++(rcount[roll - 1]);
		++(*n);
	}
}

unsigned int ecdf_distance(unsigned int sides, unsigned int* rcount) {
	unsigned int cumulative = 0, ideal_cumulative, ideal_step, i, distance, tdist;

	for(i = 0; i < sides; ++i) {
		cumulative += rcount[i];
	}

	ideal_cumulative = cumulative;
	ideal_step = ideal_cumulative / sides;
	distance = 0;

	for(i = 0; i < sides; ++i) {
		cumulative -= rcount[i];
		ideal_cumulative -= ideal_step;
		tdist = (cumulative >= ideal_cumulative) ? (cumulative - ideal_cumulative) : (ideal_cumulative - cumulative);
		if(tdist > distance) distance = tdist;
	}

	return distance;
}

int cmpuint(const void* a, const void* b) {
	return *(unsigned int*)a - *(unsigned int*)b;
}

void ecdf_gen_mc_table(unsigned int sides, unsigned int n, unsigned int count, unsigned int* table) {
	unsigned int rcount[sides];
	unsigned int i, j;
	
	for(i = 0; i < count; ++i) {
		memset(rcount, 0, sides * sizeof(unsigned int));

		for(j = 0; j < n; ++j) {
			++(rcount[rand() % sides]);
		}
		
		table[i] = ecdf_distance(sides, rcount);
	}

	qsort(table, count, sizeof(unsigned int), cmpuint);
}

bool find_in_sorted_array(unsigned int val, unsigned int count, unsigned int* array, unsigned int* low, unsigned int* high) {
	if(array[0] > val) {
		*low = *high = 0;
		return false;
	} else if(array[count - 1] < val) {
		*low = *high = count - 1;
		return false;
	}
	
	unsigned int mid;
	
	*low = 0;
	*high = count - 1;

	while(*high > *low) {
		mid = *low + (*high - *low) / 2;

		if(array[mid] == val) {
			*low = *high = mid;
			while(*low < count && array[*low] == val) --(*low);
			++(*low);
			while(*high < count && array[*high] == val) ++high;
			--(*high);
			return true;
		} else if(array[mid] > val) {
			*high = mid;
		} else {
			*low = mid;
		}
	}

	return false;
}

double chisq_test(unsigned int sides, unsigned int n, unsigned int* rcount) {
	unsigned int ideal = n / sides;
	double chisq = 0.0;

	for(unsigned int i = 0; i < sides; ++i) {
		chisq += (rcount[i] - ideal) * (rcount[i] - ideal);
	}
	chisq /= (double)ideal;
	
	return 1.0 - gsl_sf_gamma_inc_P((double)(sides - 1) / (double)2, chisq / (double)2);
}

void confidence_test(unsigned int sides, unsigned int n, unsigned int* rcount) {
	double f, amp, ideal = 1.0 / (double)sides;
	bool anomalies = false;
	
	for(unsigned int i = 0; i < sides; ++i) {
		f = (double)rcount[i] / (double)n;
		amp = 2.575 * sqrt(f * (1.0 - f) / (double)n);
		if(ideal > f + amp) {
			printf(" %d-", i+1);
			anomalies = true;
		} else if(ideal < f - amp) {
			printf(" %d+", i+1);
			anomalies = true;
		}			
	}

	if(!anomalies) {
		printf(" OK");
	}
}
