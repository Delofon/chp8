#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include "vm.h"

#define TARGET_HZ 60
#ifndef FRAMELIM
#define FRAMELIM 60
#endif

int input();
int blockinginput();
void pabeep();
uint8_t randint();

#endif

