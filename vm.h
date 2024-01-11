#ifndef VM_H
#define VM_H

#include <stdint.h>

#define STACK_SIZE 64

typedef struct _vm_t
{
    uint16_t stack[STACK_SIZE];
    uint8_t *mem;
    uint8_t V[16];
    uint16_t I;
    uint16_t PC;
    uint16_t SP;
    uint8_t halt;

    uint8_t delay;
    uint8_t sound;
} vm_t;

typedef enum
{
    ST_OK = 0,
    ST_STACKOVERFLOW,
    ST_STACKUNDERFLOW
} status;

uint16_t pop(vm_t *vm);
void push(vm_t *vm, uint16_t val);
status step(vm_t *vm);

#endif

