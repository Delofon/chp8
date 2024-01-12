#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include "vm.h"

#define TARGET_HZ 60

int input();
void draw(vm_t *vm, int x, int y, int n);
void pabeep();
uint8_t randint();

#endif

