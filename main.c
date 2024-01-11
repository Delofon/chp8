#include <sys/stat.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

#include "main.h"
#include "vm.h"
#include "sound.h"

void usage()
{
    printf("usage: chp8 <program>\n");
}

pa_simple *s;

int main(int argc, char **argv)
{
    if(argc == 1)
    {
        usage();
        return 0;
    }

    FILE *progfile = fopen(argv[1], "rb");
    if(!progfile)
    {
        fprintf(stderr, "error: could not open file \"%s\": %s\n", argv[1], strerror(errno));
        return 1;
    }

    vm_t vm =
    {
        .screen = { 0 },
        .stack = { 0 },
        .V = { 0 },
        .I = 0,
        .PC = 0x0200,
        .SP = 0,
        .halt = false,

        .delay = 0,
        .sound = 0
    };

    vm.mem = malloc(4096);
    if(!vm.mem)
    {
        fprintf(stderr, "error: could not allocate memory: %s\n", strerror(errno));
        return 2;
    }

    loadsprites(&vm);

    fread(vm.mem+0x0200, 1, 0x0fff, progfile);
    if(ferror(progfile))
    {
        fprintf(stderr, "error: could not read program file: %s\n", strerror(errno));
        return 3;
    }

    fclose(progfile);

    //FILE *memdump = fopen("memdump", "wb");
    //fwrite(vm.mem, 1, 4096, memdump);

    int paerror = 0;
    s = pabegin(&paerror);
    if(paerror != PA_OK)
    {
        fprintf(stderr, "error: could not init pulseaudio: %s\n", pa_strerror(paerror));
        return 4;
    }

    //initscr();
    
    noecho();
    nodelay(stdscr, true);

    clock_t target = CLOCKS_PER_SEC / TARGET_HZ;

    uint64_t frame = 0;

    while(!vm.halt)
    {
        //if(frame == 300) vm.halt = 1;

        clock_t start = clock();
        step(&vm);

        drawscr(&vm);
        refresh();

        while(clock() - start < target);
        frame++;
    }

    endwin(); 

    printf("registers: ");
    for(int i = 0; i < 16; i++)
    {
        printf("%d ", vm.V[i]);
    }
    printf("\nPC: %d\n", vm.PC);
    printf("SP: %d\n", vm.SP);
    printf("I:  %d\n", vm.I);

    paend(s);
    free(vm.mem);
    return 0;
}

void pabeep()
{
    paplay(s, 440, 1000);
}

int input()
{
    const int mapping[16] =
    { 
        'X',
        '1',
        '2',
        '3',
        'Q',
        'W',
        'E',
        'A',
        'S',
        'D',
        'Z',
        'C',
        '4',
        'R',
        'F',
        'V'
    };
    int ch = getch();

    for(int i = 0; i < 16; i++)
    {
        if(ch == mapping[i]) 
            return i;
    }

    return -1;
}

int coordtoi(int x, int y)
{
    if(x < 0) return -1;
    if(x > SCREEN_WIDTH) return -1;
    if(y < 0) return -1;
    if(y > SCREEN_HEIGHT) return -1;

    return x + y * SCREEN_WIDTH;
}

void itocoord(int i, int *x, int *y)
{
    *x = i % SCREEN_WIDTH;
    *y = i / SCREEN_WIDTH;
}

void draw(vm_t *vm, int x, int y, int n)
{
    uint8_t *sprite = vm->mem + (15*n);
    vm->V[15] = 0;

    for(int v = 0; v < 15; v++)
    {
        for(int u = 0; u < 8; u++)
        {
            uint8_t bit = (sprite[v] >> (7 - u)) & 0x0001;
            int i = coordtoi(x+u, y+v);
            if(i == -1) continue;

            if(vm->screen[i] && bit) vm->V[15] = 1;
            vm->screen[i] ^= bit;
        }
    }
}

void drawscr(vm_t *vm)
{
    clear();
    for(int i = 0; i < SCREEN_SIZE; i++)
    {
        int x, y;
        itocoord(i, &x, &y);
        move(y, x);
        if(vm->screen[i]) printw("0");
    }
}

void loadsprites(vm_t *vm)
{
    vm->mem[0] = 0b00011000;
    vm->mem[1] = 0b00100100;
    vm->mem[2] = 0b00100100;
    vm->mem[3] = 0b00100100;
    vm->mem[4] = 0b00011000;

    vm->mem[15] = 0b00010000;
    vm->mem[16] = 0b00110000;
    vm->mem[17] = 0b00010000;
    vm->mem[18] = 0b00010000;
    vm->mem[19] = 0b00111000;
}

