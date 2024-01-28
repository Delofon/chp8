#ifndef SCREEN_H
#define SCREEN_H

#include "vm.h"

typedef struct
{
    void (*init)();
    void (*end)();

    void (*draw)(vm_t *vm);
    void (*drawtext)(int y, int x, const char *format, ...);

    int (*input)();
} screen_t;

#endif

