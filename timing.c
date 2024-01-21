#include <time.h>

#include "timing.h"

timing_t now()
{
    return clock();
}

timing_t hztotiming(int hz)
{
    if(hz == 0)
        return 0;
    return CLOCKS_PER_SEC / hz;
}

int timingtos(timing_t tnow)
{
    return tnow / CLOCKS_PER_SEC;
}

// note:
// struct timespec ts;
// clock_gettime(CLOCK_MONOTONIC, &ts);
// long start = ts.tv_nsec; long target = 1000000000l / TARGET_HZ;
// long now = ts.tv_nsec;
// usleep((start + target - now) / 1000);

// FIXME: find something better as this has 100% cpu load
// using timespec as described above produces weird issues
// clock() combined with usleep does not produce desired results
void sleepuntil(timing_t start, timing_t nap)
{
    while(now() - start < nap);
}

