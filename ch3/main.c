#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "emulator.h"
#include "emulator_function.h"
#include "instruction.h"

/* メモリは1MB */
#define MEMORY_SIZE (1024 * 1024)

char* registers_name[] = {
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"
};

static void read_binary(Emulator* emu, const char* filename)
{
    FILE* binary;

    binary = fopen(filename, "rb");

    if (binary == NULL)
    {
        printf("cannot open file %s\n", filename);
        exit(1);
    }

    // Emulatorのメモリにバイナリファイルの内容を512バイトコピーする
    fread(emu->memory + 0x7c00, 1, 0x200, binary);
    fclose(binary);
}

/**
 * 汎用レジスタとプログラムカウンタの値を標準出力に出力する
 */
static void dump_registers(Emulator* emu)
{
    int i;

    for (i = 0; i < REGISTERS_COUNT; i++)
    {
        printf("%s = %08x\n,", registers_name[i], get_register32(emu, i));
    }

    printf("EIP = %08x\n", emu->eip);
}

/* エミュレータを作成する */
static Emulator* create_emu(size_t size, uint32_t eip, uint32_t esp)
{
    Emulator* emu = malloc(sizeof(Emulator));
    emu->memory = malloc(size);

    // 汎用レジスタの初期値を全て0にする
    memset(emu->registers, 0, sizeof(emu->registers));

    // レジスタの初期値を指定された物にする
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

int main(int argc, char* argv[])
{
    Emulator* emu;

    if (argc != 2)
    {
        printf("usage: px86 filename\n");
        return 1;
    }

    // 命令セットの初期化を行う
    init_instructions();

    // EIPが0, ESPが0x7C00の状態のエミュレータを作る
    emu = create_emu(MEMORY_SIZE, 0x7c00, 0x7c00);

    // 引数で与えられたバイナリを読み込む
    read_binary(emu, argv[1]);

    while (emu->eip < MEMORY_SIZE)
    {
        uint8_t code = get_code8(emu, 0);
        // 現在のプログラムカウンタと実行されるバイナリを出力する
        printf("EIP = %X, ESP = %X, EBP = %X, Code = %02X, StackHead = %X\n", emu->eip, emu->registers[ESP], emu->registers[EBP], code, get_memory32(emu, emu->registers[ESP]));

        if (instructions[code] == NULL)
        {
            // 実装されていない命令が来たらVMを終了する
            printf("\n\nNot Implemented: %x\n", code);
            break;
        }

        // 命令の実行
        instructions[code](emu);

        // EIPが0になったらプログラム終了
        // メモ：
        //   - 0番地へのjump命令によってeip=0にすることができる
        if (emu->eip == 0x00)
        {
            printf("\n\nend of program. \n\n");
            break;
        }
    }

    dump_registers(emu);
    destroy_emu(emu);
    return 0;
}