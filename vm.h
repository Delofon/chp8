#ifndef VM_H
#define VM_H

#include <stdint.h>

typedef struct _vm_t
{
    uint8_t V[16];
    uint16_t I;
    uint16_t PC;
    uint16_t SP;
    uint8_t halt;
} vm_t;

#endif

