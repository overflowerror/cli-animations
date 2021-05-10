#include "utils.h"

#include <time.h>
#include <stdlib.h>

void initRandom() {
    srandom(time(NULL));
}

int randomInt() {
    return random();
}

float randomFloat() {
    return random() * 1.0 / RAND_MAX;
}
