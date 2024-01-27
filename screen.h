#ifndef SCREEN_H
#define SCREEN_H

#include "vm.h"

typedef struct
{
    void (*screeninit)();
    void (*screenend)();

    void (*screendraw)(vm_t *vm);
    void (*screendrawtext)(int y, int x, const char *format, ...);

    int (*screeninput)();
} screen_t;

#endif

