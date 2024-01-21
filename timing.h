#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>
#include <time.h>

typedef clock_t timing_t;

timing_t now();
timing_t hztotiming(int hz);
int timingtos(timing_t tnow);
void sleepuntil(timing_t start, timing_t nap);

#endif

