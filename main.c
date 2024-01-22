#include <pulse/simple.h>
#include <pulse/error.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <locale.h>

#include "main.h"
#include "vm.h"
#include "timing.h"
#include "sound.h"
#include "screen.h"

void drawscr(vm_t *vm);
void loadfont(vm_t *vm);

void usage()
{
    printf("Usage: chp8 [options] <program>\n");
    printf("Note: unicode support is assumed in HIRES graphics.\n");
    printf("Options:\n");
    printf("    -e <extensions>\n");
    printf("        Choose CHIP-8 extensions. Valid arguments are: CHIP8, SCHIP, XOCHIP. Default: CHIP8\n");
    printf("    -v <video backend>\n");
    printf("        Choose video backend. Valid arguments are: ncurses, sdl. Default: ncurses\n");
    printf("    -a <audio backend>\n");
    printf("        Choose audio backend. Valid arguments are: pulse, sdl. Default: pulse\n");
}

pa_simple *s = 0;

int main(int argc, char **argv)
{
    if(argc == 1)
    {
        usage();
        return 0;
    }

    setlocale(LC_ALL, "");

    extensions_t extensions = CHIP8;
    video_t video = V_NCURSES;
    audio_t audio = A_PULSE;
    int opt;
    // TODO: use getopt_long
    while((opt = getopt(argc, argv, "e:v:a:")) != -1)
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
                        return EXIT_BAD_ARGS;
                }
                break;
            case 'v':
                if(strcmp(optarg, "ncurses") == 0)
                    video = V_NCURSES;
                else if(strcmp(optarg, "sdl") == 0)
                    video = V_SDL;
                else
                {
                        fprintf(stderr, "error: unrecognized video backend name: %s\n", optarg);
                        return EXIT_BAD_ARGS;
                }
                break;
            case 'a':
                if(strcmp(optarg, "pulse") == 0)
                    audio = A_PULSE;
                else if(strcmp(optarg, "sdl") == 0)
                    audio = A_SDL;
                else
                {
                        fprintf(stderr, "error: unrecognized audio backend name: %s\n", optarg);
                        return EXIT_BAD_ARGS;
                }
                break;
            default:
                fprintf(stderr, "error: unrecognized option %c.\n", opt);
                return EXIT_BAD_OPTION;
                break;
        }
    }

    FILE *progfile = fopen(argv[optind], "rb");
    if(!progfile)
    {
        fprintf(stderr, "error: could not open file \"%s\": %s\n", argv[optind], strerror(errno));
        return EXIT_BAD_FILE;
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
        .halt = 0,

        .delay = 0,
        .sound = 0,

        .redrawscreen = 0
    };

    vm.mem = malloc(MEMORY_SIZE);
    if(!vm.mem)
    {
        fprintf(stderr, "error: could not allocate memory: %s\n", strerror(errno));
        return EXIT_BAD_MALLOC;
    }
    memset(vm.mem, 0, MEMORY_SIZE);

    loadfont(&vm);

    fread(vm.mem+0x0200, 1, MEMORY_SIZE-0x0200, progfile);
    if(ferror(progfile))
    {
        fprintf(stderr, "error: could not read program file: %s\n", strerror(errno));
        return EXIT_BAD_FREAD;
    }

    fclose(progfile);

    int paerror = 0;
    s = pabegin(&paerror);
    if(paerror != PA_OK)
    {
        fprintf(stderr, "error: could not init pulseaudio: %s\n", pa_strerror(paerror));
        return EXIT_BAD_PULSE;
    }

    screeninit();

    const timing_t target = hztotiming(TARGET_HZ);
    timing_t frametime = 1;

    uint64_t frame = 0;

    while(!vm.halt)
    {
        if(frame == FRAME_LIM) vm.halt = 1;

        timing_t start = now();

        status_t st = step(&vm);

        if(st != ST_OK)
        {
            screenend();
            
            fprintf(stderr, "error: virtual machine encountered an error: %s\n", sttocstr(st));
            fprintf(stderr, "error occured at PC with op: 0x%04x 0x%04x\n", vm.PC, vm.op);
            fprintf(stderr, "extensions used: %s\n", exttocstr(vm.extensions));
            memdump(&vm);
            return EXIT_VM_ERROR;
        }

        if(vm.redrawscreen)
        {
            screendrawtext(49, 0, "delay timer: %d\n", vm.delay);
            screendrawtext(50, 0, "framerate: %lld\n", CLOCKS_PER_SEC / frametime);
            screendrawtext(51, 0, "frame: %lu\n", frame);
            screendraw(&vm);
            vm.redrawscreen = 0;
        }

        sleepuntil(start, target);
        frametime = now() - start;
        frame++;
    }

    screenend();

#ifdef DEBUG
    memdump(&vm);

    printf("VM state at the end of execution\n--------------------\n");
    printf("registers: ");
    for(int i = 0; i < 16; i++)
    {
        printf("%d ", vm.V[i]);
    }
    printf("\nPC: 0x%04x\n", vm.PC);
    printf("SP: 0x%04x\n", vm.SP);
    printf("I: 0x%04x\n", vm.I);
#endif

    paend(s);
    free(vm.mem);
    return 0;
}

// TODO: maybe move to sound.c?
void pabeep()
{
    paplay(s, 440, 1000);
}

// TODO: maybe open the file once at startup and then
// read&write to it without reopening it each time?
void save(uint8_t *registers, int x)
{
    FILE *regfile = fopen("./registers", "wb");
    if(!regfile)
    {
        // unable to open a file but does not deserve a crash
        // TODO: maybe set something in the vm state to indicate a runtime error?
        return;
    }

    fwrite(registers, x, 1, regfile);
    fclose(regfile);
}

void load(uint8_t *registers, int x)
{
    FILE *regfile = fopen("./registers", "rb");
    if(!regfile)
    {
        // does not deserve a crash, assume all 0
        memset(registers, 0, x);
        return;
    }

    fread(registers, x, 1, regfile);
    fclose(regfile);
}

int8_t input()
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

    int ch = screeninput();

    // FIXME: this looks awful
    if(ch == NOINP_KEYCODE)
        return NOINP_KEYCODE;
    else if(ch == 'h')
        return HALT_KEYCODE;
    else if(ch == 'm')
        return MEMDUMP_KEYCODE;
    else if(ch == 'p')
        return REFRESH_KEYCODE;

    for(int8_t i = 0; i < 16; i++)
    {
        if(ch == mapping[i]) 
            return i;
    }

    return NOINP_KEYCODE;
}

int8_t blockinginput()
{
    int inp;
    timing_t start = now();

    do
    {
        inp = input();
        sleepuntil(start, hztotiming(TARGET_HZ));
    } while(inp == NOINP_KEYCODE);

    return inp;
}

//void showinp(int inp)
//{
//    if(inp != NOINP_KEYCODE)
//        mvprintw(50, 0, "%c", inp+'0');
//    else
//        mvprintw(50, 0, " ");
//}

void loadfont(vm_t *vm)
{
    // lores
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

    // TODO: hires font
    // hires
    // 0
    //vm->mem[80]  = 0b00000000; vm->mem[81]  = 0b00000000;
    //vm->mem[82]  = 0b00000000; vm->mem[83]  = 0b00000000;
    //vm->mem[84]  = 0b00000000; vm->mem[85]  = 0b00000000;
    //vm->mem[86]  = 0b00000000; vm->mem[87]  = 0b00000000;
    //vm->mem[88]  = 0b00000000; vm->mem[89]  = 0b00000000;
    //vm->mem[90]  = 0b00000000; vm->mem[91]  = 0b00000000;
    //vm->mem[92]  = 0b00000000; vm->mem[93]  = 0b00000000;
    //vm->mem[94]  = 0b00000000; vm->mem[95]  = 0b00000000;
    //vm->mem[96]  = 0b00000000; vm->mem[97]  = 0b00000000;
    //vm->mem[98]  = 0b00000000; vm->mem[99]  = 0b00000000;
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

void memdump(vm_t *vm)
{
    FILE *memdumpf = fopen("memdump", "wb");
    fwrite(vm->mem, 1, MEMORY_SIZE, memdumpf);
    fclose(memdumpf);
}

int itocoord(int i, int *x, int *y, int width, int size)
{
    if(i < 0) return 0;
    if(i >= size) return 0;

    *x = i % width;
    *y = i / width;

    return 1;
}

int coordtoi(int x, int y, int width, int height)
{
    if(x < 0) return -1;
    if(x >= width) return -1;
    if(y < 0) return -1;
    if(y >= height) return -1;

    return x + y * width;
}

