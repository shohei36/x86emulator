#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <stdint.h>

enum Register { 
    /**
     * アキュムレータ
     * 値を蓄積する
     */
    EAX,
    /**
     * カウンタ
     * 文字列の添字やループの回数を数える
     */
    ECX,
    /**
     * データ
     * I/O装置の番地を記憶する
     */
    EDX,
    /**
     * ベース
     * メモリ番地を記憶する
     */
    EBX,
    /**
     * スタックポインタ
     * スタックの先頭を指す
     */
    ESP,
    /**
     * ベースポインタ
     * スタック上の何らかのデータを指す
     */
    EBP,
    /**
     * ソースインデックス
     * 入力データの添字を記憶する
     */
    ESI,
    /**
     * デスティネーションインデックス
     * 出力データの添字を記録する
     */
    EDI,
    REGISTERS_COUNT 
};

typedef struct {
    /* 汎用レジスタ */
    uint32_t registers[REGISTERS_COUNT];

    /**
     * フラグレジスタ
     * 演算結果やCPUの状態などを表す
     */
    uint32_t eflags;

    /* メモリ(バイト列) */
    uint8_t* memory;

    /**
     * 命令ポインタ
     * 現在実行中の命令の番地を記憶する
     */
    uint32_t eip;
} Emulator;

#endif