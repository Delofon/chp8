#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include "vm.h"

#ifndef TARGET_HZ
#define TARGET_HZ 60
#endif
#ifndef FRAME_LIM
#define FRAME_LIM 60
#endif
#define GETCH_HZ 15

int8_t input();
int8_t blockinginput();
void showinp(int inp);
void pabeep();
uint8_t randint();
void memdump(vm_t *vm);

#endif

