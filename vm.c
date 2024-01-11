#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "vm.h"

uint16_t pop(vm_t *vm)
{
    if(vm->SP == 0) return UINT16_MAX;
    return vm->stack[--vm->SP];
}

void push(vm_t *vm, uint16_t val)
{
    vm->stack[vm->SP++] = val;
}

status step(vm_t *vm)
{
    uint8_t op  = vm->mem[vm->PC];
    uint8_t hn  = (op & 0xf000) >> 2;
    uint8_t ln  = (op & 0x00ff);
    uint8_t n   = (op & 0x000f);
    uint8_t nnn = (op & 0x0fff);
    uint8_t x   = (op & 0x0f00) >> 2;
    uint8_t y   = (op & 0x00f0) >> 1;

    printf("op hn ln: 0x%04x 0x%02x 0x%02x\n", op, hn, ln);

    switch(hn)
    {
        case 0x00:
            if(ln == 0x00) vm->halt = 1;
            if(ln == 0xe0) memset(vm->screen, 0, sizeof(vm->screen));
            if(ln == 0xee) 
            {
                vm->PC = pop(vm);
                if(vm->PC == UINT16_MAX) return ST_STACKUNDERFLOW;
            }
            break;
        case 0x10:
            vm->PC = nnn;
            break;
        case 0x20:
            if(vm->SP == STACK_SIZE) return ST_STACKOVERFLOW;
            push(vm, vm->PC);
            vm->PC = nnn;
            break;
        case 0x30:
            if(vm->V[x] == ln) vm->PC++;
            break;
        case 0x40:
            if(vm->V[x] != ln) vm->PC++;
            break;
        case 0x50:
            if(vm->V[x] == vm->V[y]) vm->PC++;
            break;
        case 0x60:
            vm->V[x] = ln;
            break;
        case 0x70:
            vm->V[x] += ln;
            break;
        case 0x80:
            switch(n)
            {
                case 0x00:
                    vm->V[x] = vm->V[y];
                    break;
                case 0x01:
                    vm->V[x] |= vm->V[y];
                    break;
                case 0x02:
                    vm->V[x] &= vm->V[y];
                    break;
                case 0x03:
                    vm->V[x] ^= vm->V[y];
                    break;
                case 0x04:
                    if(vm->V[x] > 0 && vm->V[y] > UINT8_MAX - vm->V[x]) vm->V[15] = 1;
                    vm->V[x] += vm->V[y];
                    break;
                case 0x05:
                    if(vm->V[x] > 0 && vm->V[y] < UINT8_MAX + vm->V[x]) vm->V[15] = 1;
                    vm->V[x] -= vm->V[y];
                    break;
                case 0x06:
                    vm->V[15] = vm->V[x] & 0x0001;
                    vm->V[x] >>= 1;
                    break;
                case 0x07:
                    if(vm->V[y] > 0 && vm->V[x] < UINT8_MAX + vm->V[y]) vm->V[15] = 1;
                    vm->V[x] = vm->V[y] - vm->V[x];
                    break;
                case 0x0e:
                    vm->V[15] = vm->V[x] & 0x1000;
                    vm->V[x] <<= 1;
                    break;
            }
            break;
        case 0x90:
            if(vm->V[x] != vm->V[y]) vm->PC++;
            break;
        case 0xa0:
            vm->mem[vm->I] = nnn;
            break;
        case 0xb0:
            vm->PC = vm->V[0] + nnn;
            break;
        case 0xc0:
            vm->V[x] = rand() & ln;
            break;
        case 0xd0:
            draw(vm, vm->V[x], vm->V[y], n);
            break;
        case 0xe0:
            if(ln == 0x9e)
            {
                if(input() == vm->V[x]) vm->PC++;
            }
            else if(ln == 0xa1)
            {
                if(input() != vm->V[x]) vm->PC++;
            }
            break;
        case 0xf0:
            switch(ln)
            {
                case 0x07:
                    vm->V[x] = vm->delay;
                    break;
                case 0x0a:
                    vm->V[x] = input();
                    break;
                case 0x15:
                    vm->delay = vm->V[x];
                    break;
                case 0x18:
                    vm->sound = vm->V[x];
                    break;
                case 0x1e:
                    vm->I += vm->V[x];
                    break;
                case 0x29:
                    vm->I = 15 * vm->V[x]; 
                    break;
                case 0x33:
                    vm->mem[vm->I+2] = vm->V[x]       % 10;
                    vm->mem[vm->I+1] = vm->V[x] / 10  % 10;
                    vm->mem[vm->I]   = vm->V[x] / 100 % 10;
                    break;
                case 0x55:
                    memcpy(vm->mem+vm->I, vm->V, 16);
                    break;
                case 0x65:
                    memcpy(vm->V, vm->mem+vm->I, 16);
                    break;
            }
            break;
    }

    if(vm->delay >  0) vm->delay--;
    if(vm->sound == 1) pabeep();
    if(vm->sound >  0) vm->sound--;

    vm->PC++;
    return ST_OK;
}

