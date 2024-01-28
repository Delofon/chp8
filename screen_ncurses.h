#ifndef SCREEN_NCURSES_H
#define SCREEN_NCURSES_H

#include "vm.h"

void nc_init();
void nc_end();

void nc_draw(vm_t *vm);
void nc_drawtext(int y, int x, const char *format, ...);

int nc_input();

#endif

