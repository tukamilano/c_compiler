#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* メモリは1MB */
#define MEMORY_SIZE (1024 * 1024)

enum Register { EAX, ECX, EBX, ESP, EBP, ESI, EDI, REGISTERS_COUNT };
char* registers_name[] = {
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};

typedef struct {
    /* 汎用レジスタ */
    uint32_t  registers[REGISTERS_COUNT];

    /* EFLAGSレジスタ */
    uint32_t eflags;

    /* メモリ(バイト列) */
    uint8_t* memory;

    /* プログラムカウンタ */
    uint32_t eip;
} Emulator;

/* エミュレータを作成する */
static Emulator* create_emu(size_t size, uint32_t eip, uint32_t esp)
{
    Emulator* emu = malloc(sizeof(Emulator));
    emu->memory = malloc(size);

    /* 汎用レジスタの初期値を全て0にする */
    memset(emu->registers, 0, sizeof(emu->registers));

    /* レジスタの初期値を指定されたものにする */
    emu->eip = eip;
    emu->registers[ESP] = esp;

    return emu;
}

/* エミュレータを破棄する */
static void destroy_emu(Emulator* emu)
{
    free(emu->memory);
    free(emu);
}

/* 汎用レジスタとプログラムカウンタの値を標準出力に出力する */
static void dump_registers(Emulator* emu)
{
    int i;

    for (i = 0; i < REGISTERS_COUNT; i++) {
        printf("%s = %08x\n", registers_name[i], emu->registers[i]);
    }

    printf("EIP = %08x\n", emu-> eip);
}

uint32_t get_code8(Emulator* emu, int index)
{
    return emu->memory[emu->eip+index];
}

int32_t get_sign_code8(Emulator* emu, int index)
{
    return (int8_t)emu->memory[emu->eip + index];
}

uint32_t get_code32(Emulator* emu, int index)
{
    int i;
    uint32_t ret = 0;

    /* リトルエンディアンでメモリの値を取得する */
    for (i = 0; i < 4; i++) {
        ret |= get_code8(emu, index + i) << (i * 8);
    }

    return ret;
}

void mov_r32_imm32(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0xB8;
    uint32_t value = get_code32(emu, 1);
    emu->registers[reg] = value;
    emu->eip += 5;
}