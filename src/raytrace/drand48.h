#ifndef DRAND48_H
#define DRAND48_H

#include <stdlib.h>

#define _m 0x100000000LL
#define _c 0xB16
#define _a 0x5DEECE66DLL

static unsigned long long seed = 1;

double drand48(void)
{
	seed = (_a * seed + _c) & 0xFFFFFFFFFFFFLL;
	unsigned int x = seed >> 16;
	return ((double)x / (double)_m);

}

void srand48(unsigned int i)
{
	seed = (((long long int)i) << 16) | rand();
}

vec3 randomInunitDisk() {
	vec3 p;
	do {
		p = 2. * vec3(drand48(), drand48(), 0) - vec3(1, 1, 0);
	} while (dot(p, p) >= 1.);
	return p;
}

#endif