#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

static const pa_sample_spec ss = {
    .format = PA_SAMPLE_S16LE,
    .rate = 44100,
    .channels = 1
};

int16_t *sine;

pa_simple *pabegin(int *paerror)
{
    sine = malloc(2 * ss.rate);

    pa_simple *s = pa_simple_new(NULL, "chp8sine", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, paerror);
    return s;
}

int paplay(pa_simple *s, int freq, int duration)
{
    double x = 0;
    for(int i = 0; i < ss.rate; i++, x += (2 * M_PI * freq) / ss.rate)
        sine[i] = INT16_MAX * sin(x);

    int paerror = 0;
    for(int i = 0; i < duration; i++)
        pa_simple_write(s, sine, sizeof(sine), &paerror);
    return paerror;
}

int paend(pa_simple *s)
{
    int paerror = 0;
    //pa_simple_drain(s, &paerror);
    if(paerror != PA_OK) return paerror;
    pa_simple_free(s);
    return 0;
}

