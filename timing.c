#include <unistd.h>
#include <time.h>

#include "timing.h"

timing_t offset = 0;

timing_t now()
{
    return clock() + offset;
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
// typedef uint64_t usec_t;
// usec_t target = 1000000l / TARGET_HZ;
// usec_t start  = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
// usec_t now    = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
// usleep(start + target - now);

// TODO: consider using monotonic clock for timing
void sleepuntil(timing_t start, timing_t nap)
{
    // better than the while loop that was here before,
    // should still probably use monotonic clock instead
    // maybe don't abuse now() calls and instead pass the
    // frametime from main() to step() and from step()
    // further down to input() and etc?
    (void) start;
    offset += nap;
    usleep(nap * CLOCKS_PER_USEC);
}

