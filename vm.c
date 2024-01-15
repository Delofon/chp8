#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "vm.h"
#include "timing.h"

uint16_t pop(vm_t *vm)
{
    if(vm->SP == 0) return UINT16_MAX;
    return vm->stack[--vm->SP];
}

void push(vm_t *vm, uint16_t val)
{
    vm->stack[vm->SP++] = val;
}

char *sttocstr(status_t st)
{
    switch(st)
    {
        case ST_OK:
            return "ST_OK";
            break;

        case ST_STACKOVERFLOW:
            return "ST_STACKOVERFLOW";
            break;
        case ST_STACKUNDERFLOW:
            return "ST_STACKUNDERFLOW";
            break;
        case ST_SEGFAULT:
            return "ST_SEGFAULT";
            break;

        case ST_QUIRK_UNDEFINED:
            return "ST_QUIRK_UNDEFINED";
            break;
        case ST_OP_UNDEFINED:
            return "ST_OP_UNDEFINED";
            break;
    }
    return "ST_UNKNOWN";
}

char *exttocstr(extensions_t ext)
{
    switch(ext)
    {
        case CHIP8:
            return "CHIP8";
            break;
        case SCHIP:
            return "SCHIP";
            break;
        case XOCHIP:
            return "XOCHIP";
            break;
    }
    return "UNKNOWN";
}

uint8_t testaddoverflow(uint8_t a, uint8_t b)
{
    return b > UINT8_MAX - a;
}
uint8_t testsubunderflow(uint8_t a, uint8_t b)
{
    return a < b;
}

uint8_t testsegfault(uint16_t memptr, vm_t *vm)
{
    if(vm->mem+memptr >= vm->mem + MEMORY_SIZE)
        return 0;

    return 1;
}

status_t step(vm_t *vm)
{
    if(!testsegfault(vm->PC, vm))
        return ST_SEGFAULT;

    uint16_t op   = (vm->mem[vm->PC] << 8) | (vm->mem[vm->PC+1]);
    uint8_t  hn   = (op & 0xf000) >> 8;
    uint8_t  ln   = (op & 0x00ff);
    uint8_t  n    = (op & 0x000f);
    uint16_t nnn  = (op & 0x0fff);
    uint8_t  x    = (op & 0x0f00) >> 8;
    uint8_t  y    = (op & 0x00f0) >> 4;

    int8_t inp = input();

    uint8_t *shift;
    uint8_t *store = &vm->V[x];

    uint8_t flag = 0;

    vm->op = op;

    if(inp == HALT_KEYCODE)
    {
        vm->halt = 1;
        return ST_OK;
    }
    else if(inp == MEMDUMP_KEYCODE)
    {
        memdump(vm);
        return ST_OK;
    }

#ifdef DEBUG
    fprintf(stderr, "========================\n");
    fprintf(stderr, "op: 0x%04x\n", op);
    fprintf(stderr, "registers: ");
    for(int i = 0; i < 16; i++)
    {
        fprintf(stderr, "%d ", vm->V[i]);
    }
    fprintf(stderr, "\nnnn: 0x%04x %d\n", nnn, nnn);
    fprintf(stderr, "PC: 0x%04x\n", vm->PC);
    fprintf(stderr, "SP: 0x%04x\n", vm->SP);
    fprintf(stderr, "I: 0x%04x\n", vm->I);
#endif

#ifdef TESTNBLINP
    showinp(inp);
#endif

    switch(hn)
    {
        case 0x00:
            if(ln == 0xe0)
                memset(vm->screen, 0, sizeof(vm->screen));
            else if(ln == 0xee)
            {
                if(vm->SP == 0) return ST_STACKUNDERFLOW;
                vm->PC = pop(vm);
            }
            else if(vm->extensions > CHIP8)
            {
                if(ln == 0xff)
                    vm->graphicsmode = HIRES;
                else if(ln == 0xfe)
                    vm->graphicsmode = LORES;
                else if((ln & 0xf0) == 0xc0);
                    // TODO
                else if(ln == 0xfb);
                    // TODO
                else if(ln == 0xfc);
                    // TODO
                else if(ln == 0xfd)
                    vm->halt = 1;
                else
                    return ST_OP_UNDEFINED;
            }
            else
                return ST_OP_UNDEFINED;
            break;
        case 0x10:
            vm->PC = nnn - 2;
            break;
        case 0x20:
            if(vm->SP == STACK_SIZE) return ST_STACKOVERFLOW;
            push(vm, vm->PC);
            vm->PC = nnn - 2;
            break;
        case 0x30:
            if(vm->V[x] == ln) vm->PC+=2;
            break;
        case 0x40:
            if(vm->V[x] != ln) vm->PC+=2;
            break;
        case 0x50:
            if(vm->V[x] == vm->V[y]) vm->PC+=2;
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
                    if(vm->extensions == CHIP8) vm->V[15] = 0;
                    vm->V[x] |= vm->V[y];
                    break;
                case 0x02:
                    if(vm->extensions == CHIP8) vm->V[15] = 0;
                    vm->V[x] &= vm->V[y];
                    break;
                case 0x03:
                    if(vm->extensions == CHIP8) vm->V[15] = 0;
                    vm->V[x] ^= vm->V[y];
                    break;
                case 0x04:
                    flag = testaddoverflow(vm->V[x], vm->V[y]);
                    vm->V[x] += vm->V[y];
                    vm->V[15] = flag;
                    break;
                case 0x05:
                    flag = !testsubunderflow(vm->V[x], vm->V[y]);
                    vm->V[x] -= vm->V[y];
                    vm->V[15] = flag;
                    break;
                case 0x06:
                    if(vm->extensions == CHIP8)
                        shift = &vm->V[y];
                    else if(vm->extensions == SCHIP)
                        shift = &vm->V[x];
                    else
                        return ST_QUIRK_UNDEFINED;

                    vm->V[15] = *shift & 0x01;
                    *store = *shift >> 1;
                    break;
                case 0x07:
                    flag = !testsubunderflow(vm->V[y], vm->V[x]);
                    vm->V[x] = vm->V[y] - vm->V[x];
                    vm->V[15] = flag;
                    break;
                case 0x0e:
                    if(vm->extensions == CHIP8)
                        shift = &vm->V[y];
                    else if(vm->extensions == SCHIP)
                        shift = &vm->V[x];
                    else
                        return ST_QUIRK_UNDEFINED;

                    vm->V[15] = *shift >> 7;
                    *store = *shift << 1;
                    break;
                default:
                    return ST_OP_UNDEFINED;
                    break;
            }
            break;
        case 0x90:
            if(vm->V[x] != vm->V[y]) vm->PC+=2;
            break;
        case 0xa0:
            vm->I = nnn;
            break;
        case 0xb0:
            vm->PC = vm->V[0] + nnn - 2;
            break;
        case 0xc0:
            vm->V[x] = randint() & ln;
            break;
        case 0xd0:
            if(vm->extensions > CHIP8 && n == 0)
                n = 16;

            if(!testsegfault(vm->I, vm))
                return ST_SEGFAULT;
            if(!testsegfault(vm->I+n, vm))
                return ST_SEGFAULT;

            draw(vm, vm->V[x], vm->V[y], n);
            break;
        case 0xe0:
            if(ln == 0x9e)
            {
                if(inp == vm->V[x]) vm->PC+=2;
            }
            else if(ln == 0xa1)
            {
                if(inp != vm->V[x]) vm->PC+=2;
            }
            else
                return ST_OP_UNDEFINED;
            break;
        case 0xf0:
            switch(ln)
            {
                case 0x07:
                    vm->V[x] = vm->delay;
                    break;
                case 0x0a:
                    vm->V[x] = blockinginput();
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
                    vm->I = vm->V[x]*5; 
                    break;
                case 0x30:
                    if(vm->extensions == CHIP8)
                        return ST_OP_UNDEFINED;
                    // TODO
                    break;
                case 0x33:
                    if(!testsegfault(vm->I, vm))
                        return ST_SEGFAULT;

                    vm->mem[vm->I+2] = vm->V[x]       % 10;
                    vm->mem[vm->I+1] = vm->V[x] / 10  % 10;
                    vm->mem[vm->I]   = vm->V[x] / 100 % 10;
                    break;
                case 0x55:
                    if(!testsegfault(vm->I, vm))
                        return ST_SEGFAULT;
                    if(!testsegfault(vm->I+x, vm))
                        return ST_SEGFAULT;

                    memcpy(vm->mem+vm->I, &vm->V, x+1);
                    if(vm->extensions == CHIP8)
                        vm->I += x + 1;
                    else if(vm->extensions == SCHIP);
                    else
                        return ST_QUIRK_UNDEFINED;
                    break;
                case 0x65:
                    if(!testsegfault(vm->I, vm))
                        return ST_SEGFAULT;
                    if(!testsegfault(vm->I+x, vm))
                        return ST_SEGFAULT;

                    memcpy(&vm->V, vm->mem+vm->I, x+1);
                    if(vm->extensions == CHIP8)
                        vm->I += x + 1;
                    else if(vm->extensions == SCHIP);
                    else
                        return ST_QUIRK_UNDEFINED;
                    break;
                case 0x75:
                    if(vm->extensions == CHIP8)
                        return ST_OP_UNDEFINED;
                    // TODO
                    break;
                case 0x85:
                    if(vm->extensions == CHIP8)
                        return ST_OP_UNDEFINED;
                    // TODO
                    break;
                default:
                    return ST_OP_UNDEFINED;
                    break;
            }
            break;
        default:
            return ST_OP_UNDEFINED;
            break;
    }

    timing_t tnow = now();

    if(tnow - vm->dstart >= hztotiming(DELAY_HZ))
    {
        vm->dstart = tnow;
        if(vm->sound == 1) pabeep();
        if(vm->delay >  0) vm->delay--;
        if(vm->sound >  0) vm->sound--;
    }

    vm->PC+=2;
    if(vm->PC > 0x0ffe) vm->halt = 1;
    return ST_OK;
}

int coordtoi(int x, int y)
{
    if(x < 0) return -1;
    if(x >= SCREEN_WIDTH) return -1;
    if(y < 0) return -1;
    if(y >= SCREEN_HEIGHT) return -1;

    return x + y * SCREEN_WIDTH;
}

void draw(vm_t *vm, int x, int y, int n)
{
    if(n == 16)
        return;

    uint8_t *sprite = &vm->mem[vm->I];
    vm->V[15] = 0;
    vm->redrawscreen = 1;

    x %= SCREEN_WIDTH;
    y %= SCREEN_HEIGHT;

    for(int v = 0; v < n; v++)
    {
        for(int u = 0; u < 8; u++)
        {
            uint8_t bit = (sprite[v] >> (7 - u)) & 0x01;
            int i = coordtoi(x+u, y+v);
            if(i == -1) continue;

            if(vm->screen[i] && bit) vm->V[15] = 1;
            vm->screen[i] ^= bit;
        }
    }
}

