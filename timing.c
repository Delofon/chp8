#include <unistd.h>
#include <time.h>

#include "timing.h"

timing_t now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    usec_t now = ts.tv_sec * USEC_PER_SEC + ts.tv_nsec / 1000;
    return now;
}

timing_t hztotiming(int hz)
{
    if(hz == 0)
        return 0;
    return USEC_PER_SEC / hz;
}

int timingtos(timing_t tnow)
{
    return tnow / USEC_PER_SEC;
}

// note:
// struct timespec ts;
// clock_gettime(CLOCK_MONOTONIC, &ts);
// typedef uint64_t usec_t;
// usec_t target = 1000000l / TARGET_HZ;
// usec_t start  = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
// usec_t now    = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
// usleep(start + target - now);

void sleepuntil(timing_t start, timing_t nap)
{
    // should still probably use monotonic clock instead
    // maybe don't abuse now() calls and instead pass the
    // frametime from main() to step() and from step()
    // further down to input() and etc?
    (void) start;
    usleep(nap);
}

