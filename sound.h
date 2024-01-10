#ifndef SOUND_H
#define SOUND_H

#include <pulse/simple.h>

pa_simple *pabegin(int *paerror);
int paplay(pa_simple *s, int freq, int duration);
int paend(pa_simple *s);

#endif

