#include "include/mathlib.h"

int abs(int i) {
    return i < 0 ? -i : i;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

static unsigned int next = 1;

void srand(unsigned int seed) {
    next = seed;
}

int rand(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

