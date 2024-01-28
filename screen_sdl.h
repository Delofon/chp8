#ifndef SCREEN_SDL_H
#define SCREEN_SDL_H

#include "vm.h"

void sdl_init();
void sdl_end();

void sdl_draw(vm_t *vm);
void sdl_drawtext(int y, int x, const char *format, ...);

int sdl_input();

#endif

