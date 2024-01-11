#ifndef MAIN_H
#define MAIN_H

#include "vm.h"

#define TARGET_HZ 60
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

int input();
void draw(vm_t *vm, int x, int y, int n);
void pabeep();

#endif

