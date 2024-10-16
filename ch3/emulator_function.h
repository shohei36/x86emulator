#ifndef EMULATOR_FUNCTION_H_
#define EMULATOR_FUNCTION_H_

#include <stdint.h>

#include "emulator.h"

#define CARRY_FLAG (1)
#define ZERO_FLAG (1 << 6)
#define SIGN_FLAG (1 << 7)
#define OVERFLOW_FLAG (1 << 11)


/* memory配列の指定した番地から8ビットの値を取得する */
uint32_t get_code8(Emulator* emu, int index);

/* memory配列の指定した番地から符号付の8ビットの値を取得する */
int32_t get_sign_code8(Emulator* emu, int index);

/* memory配列の指定した番地から32ビットの値を取得する */
uint32_t get_code32(Emulator* emu, int index);

/* memory配列の指定した番地から符号付の8ビットの値を取得する */
int32_t get_sign_code32(Emulator* emu, int index);

/* index番目の8bit汎用レジスタの値を取得する */
uint8_t get_register8(Emulator* emu, int index);

/* index番目の32bit汎用レジスタの値を取得する */
uint32_t get_register32(Emulator* emu, int index);

/* index番目の32bit汎用レジスタに値を設定する */
void set_register32(Emulator* emu, int index, uint32_t value);

/* index番目の8bit汎用レジスタに値を設定する */
void set_register8(Emulator* emu, int index, uint8_t value);

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

/**
 * 減算の結果に応じてeflagsのフラグを更新する
 * sub v1, v2
 */
void update_eflags_sub(Emulator* emu, uint32_t v1, uint32_t v2, uint64_t result);

/* EFLAGの各フラグ設定用関数 */
void set_carry(Emulator* emu, int is_carry);
void set_zero(Emulator* emu, int is_zero);
void set_sign(Emulator* emu, int is_sign);
void set_overflow(Emulator* emu, int is_overflow);

/* EFLAGの各フラグ取得用関数 */
int32_t is_carry(Emulator* emu);
int32_t is_zero(Emulator* emu);
int32_t is_sign(Emulator* emu);
int32_t is_overflow(Emulator* emu);

#endif