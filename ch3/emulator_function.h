#ifndef EMULATOR_FUNCTION_H_
#define EMULATOR_FUNCTION_H_

#include <stdint.h>

#include "emulator.h"


/* memory配列の指定した番地から8ビットの値を取得する */
uint32_t get_code8(Emulator* emu, int index);

/* memory配列の指定した番地から符号付の8ビットの値を取得する */
int32_t get_sign_code8(Emulator* emu, int index);

/* memory配列の指定した番地から32ビットの値を取得する */
uint32_t get_code32(Emulator* emu, int index);

/* memory配列の指定した番地から符号付の8ビットの値を取得する */
int32_t get_sign_code32(Emulator* emu, int index);

/* index番目の32bit汎用レジスタの値を取得する */
uint32_t get_register32(Emulator* emu, int index);

/* index番目の32bit汎用レジスタに値を設定する */
void set_register32(Emulator* emu, int index, uint32_t value);

/* メモリのindex番地の8bit値を取得する */
uint32_t get_memory8(Emulator* emu, uint32_t address);

/* メモリのindex番地の32bit値を取得する */
uint32_t get_memory32(Emulator* emu, uint32_t address);

/* メモリのindex番地に8bit値を設定する */
void set_memory8(Emulator* emu, uint32_t address, uint32_t value);

/* メモリのindex番地に32bit値を設定する */
void set_memory32(Emulator* emu, uint32_t address, uint32_t value);

/* スタックに値をpushする*/
void push32(Emulator* emu, uint32_t value);

/* スタックから値をpopする */
uint32_t pop32(Emulator* emu);

#endif