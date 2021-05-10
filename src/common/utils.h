#ifndef UTILS_H
#define UTILS_H

#include "compat.h"
#include <stdio.h>
#include <stdlib.h>

void initRandom();

int randomInt();
float randomFloat();

#ifndef EXIT_PANIC
	#define EXIT_PANIC (1)
#endif

#define panic(format, ...) { cfprintf(stderr, "panic: " format "\n", #__VA_ARGS__); exit(EXIT_PANIC ); }

#endif
