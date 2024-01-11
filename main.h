#ifndef MAIN_H
#define MAIN_H

#include "vm.h"

#define TARGET_HZ 60

int input();
void draw(vm_t *vm, int x, int y, int n);
void pabeep();
void drawscr(vm_t *vm);
void loadsprites(vm_t *vm);

#endif

