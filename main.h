#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include "vm.h"

#ifndef TARGET_HZ
#define TARGET_HZ 60
#endif
#ifndef FRAME_LIM
#define FRAME_LIM -1
#endif
#define GETCH_HZ 15

typedef enum
{
    V_NCURSES,
    V_SDL
} video_t;

typedef enum
{
    A_PULSE,
    A_SDL
} audio_t;

typedef enum
{
    EXIT_OK = 0,
    EXIT_BAD_FILE,
    EXIT_BAD_MALLOC,
    EXIT_BAD_FREAD,
    EXIT_BAD_PULSE,
    EXIT_VM_ERROR,
    EXIT_BAD_ARGS,
    EXIT_BAD_OPTION
} exit_t;

void save(uint8_t *registers, int x);
void load(uint8_t *registers, int x);

int8_t input();
int8_t blockinginput();

void showinp(int inp);
void pabeep();
uint8_t randint();
void memdump(vm_t *vm);

int coordtoi(int x, int y, int width, int height);
int itocoord(int i, int *x, int *y, int width, int size);

#endif

