#ifndef SCREEN_NCURSES_H
#define SCREEN_NCURSES_H

#include "vm.h"

void nc_screeninit();
void nc_screenend();

void nc_screendraw(vm_t *vm);
void nc_screendrawtext(int y, int x, const char *format, ...);

int nc_screeninput();

#endif

