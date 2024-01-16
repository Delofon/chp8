#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <time.h>

#include "timing.h"

#define STACK_SIZE 64
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH_HIRES 128
#define SCREEN_HEIGHT_HIRES 64
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define SCREEN_SIZE_HIRES (SCREEN_WIDTH_HIRES * SCREEN_HEIGHT_HIRES)
#define MEMORY_SIZE 4096

#define NOINP_KEYCODE -1
#define HALT_KEYCODE -2
#define MEMDUMP_KEYCODE -3

#define DELAY_HZ 60

typedef enum
{
    CHIP8 = 0,
    SCHIP,
    XOCHIP
} extensions_t;

typedef enum
{
    LORES = 0,
    HIRES
} graphicsmode_t;

typedef struct _vm_t
{
    extensions_t extensions;

    uint16_t stack[STACK_SIZE];
    uint8_t screen[SCREEN_SIZE];
    uint8_t screenhr[SCREEN_SIZE_HIRES];
    uint8_t *mem;
    uint8_t V[16];
    uint16_t I;
    uint16_t PC;
    uint16_t SP;
    uint16_t op;

    graphicsmode_t graphicsmode;
    uint8_t halt;

    uint8_t delay;
    uint8_t sound;

    timing_t fstart;
    timing_t dstart;

    uint8_t redrawscreen;
} vm_t;

typedef enum
{
    ST_OK = 0,

    ST_STACKOVERFLOW,
    ST_STACKUNDERFLOW,
    ST_SEGFAULT,

    ST_QUIRK_UNDEFINED,
    ST_OP_UNDEFINED
} status_t;

uint16_t pop(vm_t *vm);
void push(vm_t *vm, uint16_t val);
status_t step(vm_t *vm);
char *sttocstr(status_t st);
char *exttocstr(extensions_t ext);
void draw(vm_t *vm, int x, int y, int n);

#endif

