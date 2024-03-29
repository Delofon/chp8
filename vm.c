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
    if(vm->SP == STACK_SIZE) return;
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
    if(!testsegfault(vm->PC+1, vm))
        return ST_SEGFAULT;

    uint16_t op   = (vm->mem[vm->PC] << 8) | (vm->mem[vm->PC+1]);
    uint8_t  hn   = (op & 0xf000) >> 8;
    uint8_t  ln   = (op & 0x00ff);
    uint8_t  n    = (op & 0x000f);
    uint16_t nnn  = (op & 0x0fff);
    uint8_t  x    = (op & 0x0f00) >> 8;
    uint8_t  y    = (op & 0x00f0) >> 4;

    int8_t inp = input();

    uint8_t shift;
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
    else if(inp == REFRESH_KEYCODE)
    {
        vm->redrawscreen = 1;
        return ST_OK;
    }

#ifdef FLOOD_WITH_OPS
    fprintf(stderr, "========================\n");
    fprintf(stderr, "op: 0x%04x\n", op);
    fprintf(stderr, "registers: ");
    for(int i = 0; i < 16; i++)
    {
        fprintf(stderr, "%d ", vm->V[i]);
    }
    fprintf(stderr, "\nnnn: 0x%04x %d\n", nnn, nnn);
    fprintf(stderr, "ln: 0x%02x %d\n", ln, ln);
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
            if(ln == 0x00)
            {
                // do nothing
            }
            else if(ln == 0xe0)
            {
                if(vm->graphicsmode == LORES)
                    memset(vm->screen, 0, sizeof(vm->screen));
                else
                    memset(vm->screenhr, 0, sizeof(vm->screenhr));
                vm->redrawscreen = 1;
            }
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
                    // TODO: scrolldown n - move screen up by n pixels
                else if(ln == 0xfb);
                    // TODO: scrollright - move screen left by 4 pixels
                else if(ln == 0xfc);
                    // TODO: scrollleft - move screen right by 4 pixels
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
                        shift = vm->V[y];
                    else if(vm->extensions == SCHIP)
                        shift = vm->V[x];
                    else
                        return ST_QUIRK_UNDEFINED;

                    flag      = shift & 0x01;
                    vm->V[x]  = shift >> 1;
                    vm->V[15] = flag;
                    break;
                case 0x07:
                    flag = !testsubunderflow(vm->V[y], vm->V[x]);
                    vm->V[x] = vm->V[y] - vm->V[x];
                    vm->V[15] = flag;
                    break;
                case 0x0e:
                    if(vm->extensions == CHIP8)
                        shift = vm->V[y];
                    else if(vm->extensions == SCHIP)
                        shift = vm->V[x];
                    else
                        return ST_QUIRK_UNDEFINED;

                    flag      = shift >> 7;
                    vm->V[x]  = shift << 1;
                    vm->V[15] = flag;
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
            // TODO: make this quirk disableable at runtime as some games don't support it
#ifndef DISABLEJUMPINGQUIRK
            if(vm->extensions == CHIP8)
                vm->PC = vm->V[0] + nnn - 2;
            else if(vm->extensions == SCHIP)
                vm->PC = vm->V[x] + nnn - 2;
            else
                return ST_QUIRK_UNDEFINED;
#else
            vm->PC = vm->V[0] + nnn - 2;
#endif
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
                    vm->I = vm->V[x]*20 + 80;
                    break;
                case 0x33:
                    if(!testsegfault(vm->I, vm))
                        return ST_SEGFAULT;
                    if(!testsegfault(vm->I+2, vm))
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
                    save(vm->V, x);
                    break;
                case 0x85:
                    if(vm->extensions == CHIP8)
                        return ST_OP_UNDEFINED;
                    load(vm->V, x);
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

void draw16(vm_t *vm, uint8_t *screen, int x, int y, int width, int height)
{
    uint8_t *sprite = &vm->mem[vm->I];
    vm->V[15] = 0;
    vm->redrawscreen = 1;

    x %= width;
    y %= height;

    for(int v = 0; v < 32; v+=2)
    {
        for(int u = 0; u < 16; u++)
        {
            uint8_t bit;
            if(u < 8)
                bit = (sprite[v]   >> (7  - u)) & 0x01;
            else
                bit = (sprite[v+1] >> (15 - u)) & 0x01;

            int i;
            if(!coordtoi(&i, x+u, y+v/2, width, height))
                continue;

            if(screen[i] && bit) vm->V[15] = 1;
            screen[i] ^= bit;
        }
    }
}

void drawsprite(vm_t *vm, uint8_t *screen, int x, int y, int n, int width, int height)
{
    if(vm->extensions > CHIP8 && n == 16)
    {
        draw16(vm, screen, x, y, width, height);
        return;
    }

    uint8_t *sprite = &vm->mem[vm->I];
    vm->V[15] = 0;
    vm->redrawscreen = 1;

    x %= width;
    y %= height;

    for(int v = 0; v < n; v++)
    {
        for(int u = 0; u < 8; u++)
        {
            uint8_t bit = (sprite[v] >> (7 - u)) & 0x01;

            int i;
            if(!coordtoi(&i, x+u, y+v, width, height))
                continue;

            if(screen[i] && bit) vm->V[15] = 1;
            screen[i] ^= bit;
        }
    }
}

void draw(vm_t *vm, int x, int y, int n)
{
    if(vm->graphicsmode == LORES)
        drawsprite(vm, vm->screen,   x, y, n, SCREEN_WIDTH, SCREEN_HEIGHT);
    else
        drawsprite(vm, vm->screenhr, x, y, n, SCREEN_WIDTH_HIRES, SCREEN_HEIGHT_HIRES);
}

