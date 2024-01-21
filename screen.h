#ifndef SCREEN_H
#define SCREEN_H

#include "vm.h"

#define UPPERHALF L"\u2580"
#define LOWERHALF L"\u2584"
#define FULLBLOCK L"\u2588"

void screeninit();
void screenend();

void screendraw(vm_t *vm);
void screendrawtext(int y, int x, const char *format, ...);

int screeninput();

#endif

