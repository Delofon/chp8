#include <pulse/simple.h>
#include <pulse/error.h>
#include <unistd.h>
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

void drawscr(vm_t *vm);
void loadfont(vm_t *vm);

void usage()
{
    printf("Usage: chp8 [options] <program>\n");
    printf("Options:\n");
    printf("    -e <extensions>\n");
    printf("        Choose CHIP-8 extensions. Valid arguments are: CHIP8, SCHIP, XOCHIP. Default: CHIP8\n");
}

pa_simple *s;

int main(int argc, char **argv)
{
    if(argc == 1)
    {
        usage();
        return 0;
    }

    extensions_t extensions = CHIP8;
    int opt;
    while((opt = getopt(argc, argv, "e:")) != -1)
    {
        switch(opt)
        {
            case 'e':
                if(strcmp(optarg, "CHIP8") == 0)
                    extensions = CHIP8;
                else if(strcmp(optarg, "SCHIP") == 0)
                    extensions = SCHIP;
                else if(strcmp(optarg, "XOCHIP") == 0)
                    extensions = XOCHIP;
                else
                {
                        fprintf(stderr, "error: unrecognized CHIP-8 extensions name: %s\n", optarg);
                        return 6;
                }
                break;
            default:
                fprintf(stderr, "warning: unrecognized option %c, ignoring.\n", opt);
                break;
        }
    }

    FILE *progfile = fopen(argv[optind], "rb");
    if(!progfile)
    {
        fprintf(stderr, "error: could not open file \"%s\": %s\n", argv[optind], strerror(errno));
        return 1;
    }

    vm_t vm =
    {
        .extensions = extensions,

        .screen = { 0 },
        .stack = { 0 },
        .V = { 0 },
        .I = 0,
        .PC = 0x0200,
        .SP = 0,
        .halt = false,

        .delay = 0,
        .sound = 0,

        .redrawscreen = 0
    };

    vm.mem = malloc(MEMORY_SIZE);
    if(!vm.mem)
    {
        fprintf(stderr, "error: could not allocate memory: %s\n", strerror(errno));
        return 2;
    }
    memset(vm.mem, 0, MEMORY_SIZE);

    loadfont(&vm);

    fread(vm.mem+0x0200, 1, MEMORY_SIZE-0x0200, progfile);
    if(ferror(progfile))
    {
        fprintf(stderr, "error: could not read program file: %s\n", strerror(errno));
        return 3;
    }

    fclose(progfile);

    int paerror = 0;
    s = pabegin(&paerror);
    if(paerror != PA_OK)
    {
        fprintf(stderr, "error: could not init pulseaudio: %s\n", pa_strerror(paerror));
        return 4;
    }

#ifndef DEBUG
    initscr();
#endif
    
    noecho();
    nodelay(stdscr, true);

    clock_t target = CLOCKS_PER_SEC / TARGET_HZ;

    uint64_t frame = 0;

    while(!vm.halt)
    {
#ifdef DEBUG
        if(frame == FRAMELIM) vm.halt = 1;
#endif

        clock_t start = clock();
        status_t st = step(&vm);

        if(st != ST_OK)
        {
            endwin();
            fprintf(stderr, "error: virtual machine encountered an error: %s\n", sttocstr(st));
            return 5;
        }

        if(vm.redrawscreen)
        {
            drawscr(&vm);
            refresh();
            vm.redrawscreen = 0;
        }

        while(clock() - start < target);
        frame++;
    }

    endwin(); 

#ifdef DEBUG
    FILE *memdump = fopen("memdump", "wb");
    fwrite(vm.mem, 1, MEMORY_SIZE, memdump);

    printf("====================\nVM state at the end of execution\n====================\n");
    printf("registers: ");
    for(int i = 0; i < 16; i++)
    {
        printf("%d ", vm.V[i]);
    }
    printf("\nPC: %d\n", vm.PC);
    printf("SP: %d\n", vm.SP);
    printf("I:  %d\n", vm.I);
#endif

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
        'x',
        '1',
        '2',
        '3',
        'q',
        'w',
        'e',
        'a',
        's',
        'd',
        'z',
        'c',
        '4',
        'r',
        'f',
        'v'
    };
    int ch = getch();

    for(int i = 0; i < 16; i++)
    {
        if(ch == mapping[i]) 
            return i;
    }

    return -1;
}

int blockinginput()
{
    nodelay(stdscr, false);
    int inp = input();
    nodelay(stdscr, true);
    return inp;
}

void itocoord(int i, int *x, int *y)
{
    *x = i % SCREEN_WIDTH;
    *y = i / SCREEN_WIDTH;
}

void drawscr(vm_t *vm)
{
    for(int i = 0; i < SCREEN_SIZE; i++)
    {
        int x, y;
        itocoord(i, &x, &y);
        move(y, x);
        if(vm->screen[i]) printw("0");
    }
}

void loadfont(vm_t *vm)
{
    // 0
    vm->mem[0] = 0xf0;
    vm->mem[1] = 0x90;
    vm->mem[2] = 0x90;
    vm->mem[3] = 0x90;
    vm->mem[4] = 0xf0;

    // 1
    vm->mem[5] = 0x20;
    vm->mem[6] = 0x60;
    vm->mem[7] = 0x20;
    vm->mem[8] = 0x20;
    vm->mem[9] = 0x70;

    // 2
    vm->mem[10] = 0xf0;
    vm->mem[11] = 0x10;
    vm->mem[12] = 0xf0;
    vm->mem[13] = 0x80;
    vm->mem[14] = 0xf0;

    // 3
    vm->mem[15] = 0xf0;
    vm->mem[16] = 0x10;
    vm->mem[17] = 0xf0;
    vm->mem[18] = 0x10;
    vm->mem[19] = 0xf0;

    // 4
    vm->mem[20] = 0x90;
    vm->mem[21] = 0x90;
    vm->mem[22] = 0xf0;
    vm->mem[23] = 0x10;
    vm->mem[24] = 0x10;

    // 5
    vm->mem[25] = 0xf0;
    vm->mem[26] = 0x80;
    vm->mem[27] = 0xf0;
    vm->mem[28] = 0x10;
    vm->mem[29] = 0xf0;

    // 6
    vm->mem[30] = 0xf0;
    vm->mem[31] = 0x80;
    vm->mem[32] = 0xf0;
    vm->mem[33] = 0x90;
    vm->mem[34] = 0xf0;

    // 7
    vm->mem[35] = 0xf0;
    vm->mem[36] = 0x10;
    vm->mem[37] = 0x20;
    vm->mem[38] = 0x40;
    vm->mem[39] = 0x40;

    // 8
    vm->mem[40] = 0xf0;
    vm->mem[41] = 0x90;
    vm->mem[42] = 0xf0;
    vm->mem[43] = 0x90;
    vm->mem[44] = 0xf0;

    // 9
    vm->mem[45] = 0xf0;
    vm->mem[46] = 0x90;
    vm->mem[47] = 0xf0;
    vm->mem[48] = 0x10;
    vm->mem[49] = 0xf0;

    // A
    vm->mem[50] = 0xf0;
    vm->mem[51] = 0x90;
    vm->mem[52] = 0xf0;
    vm->mem[53] = 0x90;
    vm->mem[54] = 0x90;

    // B
    vm->mem[55] = 0xe0;
    vm->mem[56] = 0x90;
    vm->mem[57] = 0xe0;
    vm->mem[58] = 0x90;
    vm->mem[59] = 0xe0;

    // C
    vm->mem[60] = 0xf0;
    vm->mem[61] = 0x80;
    vm->mem[62] = 0x80;
    vm->mem[63] = 0x80;
    vm->mem[64] = 0xf0;

    // D
    vm->mem[65] = 0xe0;
    vm->mem[66] = 0x90;
    vm->mem[67] = 0x90;
    vm->mem[68] = 0x90;
    vm->mem[69] = 0xe0;

    // E
    vm->mem[70] = 0xf0;
    vm->mem[71] = 0x80;
    vm->mem[72] = 0xf0;
    vm->mem[73] = 0x80;
    vm->mem[74] = 0xf0;

    // F
    vm->mem[75] = 0xf0;
    vm->mem[76] = 0x80;
    vm->mem[77] = 0xf0;
    vm->mem[78] = 0x80;
    vm->mem[79] = 0x80;
}

uint32_t wyhash = 0;
uint32_t wyhash32()
{
    wyhash += 0xe120fc15;
    uint64_t tmp;
    tmp = (uint64_t)wyhash * 0x4a39b70d;
    uint32_t m1 = (tmp >> 32) ^ tmp;
    tmp = (uint64_t)m1 * 0x12fad5c9;
    uint32_t m2 = (tmp >> 32) ^ tmp;
    return m2;
}

uint8_t randint()
{
    return (uint8_t)wyhash32();
}

