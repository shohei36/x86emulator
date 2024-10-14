#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "emulator.h"
#include "emulator_function.h"
#include "io.h"

#include "modrm.h"

/* x86命令の配列、opecode番目の関数がx86の
   opcodeに対応した命令となっている */
instruction_func_t* instructions[256];

static void mov_r8_imm8(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0xB0;
    set_register8(emu, reg, get_code8(emu, 1));
    emu->eip += 2;
}

static void mov_r32_imm32(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0xB8; // オペコードから0xB8を引くとレジスタが得られる 
    uint32_t value = get_code32(emu, 1);
    set_register32(emu, reg, value);
    emu->eip += 5;
}

static void mov_r8_rm8(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t rm8 = get_rm8(emu, &modrm);
    set_r8(emu, &modrm, rm8);
}

static void mov_r32_rm32(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t rm32 = get_rm32(emu, &modrm);
    set_r32(emu, &modrm, rm32);
}

static void add_rm32_r32(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t r32 = get_r32(emu, &modrm);
    uint32_t rm32 = get_rm32(emu, &modrm);
    set_rm32(emu, &modrm, rm32 + r32);
}

static void mov_rm8_r8(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t r8 = get_r8(emu, &modrm);
    set_rm8(emu, &modrm, r8);
}

static void mov_rm32_r32(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t r32 = get_r32(emu, &modrm);
    set_rm32(emu, &modrm, r32);
}

static void inc_r32(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0x40;
    set_register32(emu, reg, get_register32(emu, reg) + 1);
    emu->eip += 1;
}

/**
 * intelのマニュアルでは、push r32は50+rd と書いてある。
 * ベース値(0x50)にレジスタ番号を足したものがオペコードになる
 */
static void push_r32(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0x50;
    push32(emu, get_register32(emu, reg));
    emu->eip += 1;
}

/**
 * intelのマニュアルでは、pip r32は58+rd と書いてある。
 * ベース値(0x58)にレジスタ番号を足したものがオペコードになる
 */
static void pop_r32(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0x58;
    set_register32(emu, reg, pop32(emu));
    emu->eip += 1;
}

/**
 * 32bitの即値をスタックにプッシュします
 */
static void push_imm32(Emulator* emu)
{
    uint32_t value = get_code32(emu, 1);
    push32(emu, value);
    emu->eip += 5;
}

/**
 * 8bitの即値をスタックにプッシュします
 */
static void push_imm8(Emulator* emu)
{
    uint32_t value = get_code8(emu, 1);
    push32(emu, value);
    emu->eip += 2;
}

static void add_rm32_imm8(Emulator* emu, ModRM* modrm)
{
    uint32_t rm32 = get_rm32(emu, modrm);
    uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
    emu->eip += 1;
    set_rm32(emu, modrm, rm32 + imm8);
}

static void cmp_rm32_imm8(Emulator* emu, ModRM* modrm)
{
    uint32_t rm32 = get_rm32(emu, modrm);
    uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
    emu->eip += 1;
    uint64_t result = (uint64_t)rm32 - (uint64_t)imm8;
    update_eflags_sub(emu, rm32, imm8, result);
}

static void sub_rm32_imm8(Emulator* emu, ModRM* modrm)
{
    uint32_t rm32 = get_rm32(emu, modrm);
    uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
    emu->eip += 1;
    uint64_t result = (uint64_t)rm32 - (uint64_t)imm8;
    set_rm32(emu, modrm, result);
    update_eflags_sub(emu, rm32, imm8, result);
}

/**
 * 減算(sub)
//  * ※intelのマニュアルには 83 /5 ib が sub rm32, imm8 に対応するとある
 */
static void code_83(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    
    switch (modrm.opecode)
    {
    case 0:
        add_rm32_imm8(emu, &modrm);
        break;
    case 5:
        sub_rm32_imm8(emu, &modrm);
        break;
    case 7:
        cmp_rm32_imm8(emu, &modrm);
    default:
        printf("not implemented: 83 /%d\n", modrm.opecode);
        exit(1);
    }
}

/**
 * 32ビットの即値を、modr/mで指定されたレジスタまたはメモリ領域に書き込む機械語(オペコード0x7c)に対応
 * ※modr/mのmodとrmで指定されるnビットのレジスタまたはメモリ領域のことをrmn、
 *   regで指定されるnビットのレジスタをrn、nビットの即値をimmnと表す
 */
static void mov_rm32_imm32(Emulator* emu)
{
    emu->eip += 1; // 関数が呼ばれたときはemu->eipはオペコードを指した状態
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t value = get_code32(emu, 0); // parse_modrmから戻るとemu->eipは即値を指す
    emu->eip += 4;
    set_rm32(emu, &modrm, value); // modrmの設定に従って値を書き込む
}

/**
 * dx(i/oポートの値)をalに格納する
 */
static void in_al_dx(Emulator* emu)
{
    uint16_t address = get_register32(emu, EDX) & 0xffff;
    uint8_t value = io_in8(address);
    set_register8(emu, AL, value);
    emu->eip += 1;
}

/**
 * alの値をdx(i/oポートの番地)に出力する
 */
static void out_dx_al(Emulator* emu)
{
    uint16_t address = get_register32(emu, EDX) & 0xffff;
    uint8_t value = get_register8(emu, AL);
    io_out8(address, value);
    emu->eip += 1;
}

static void inc_rm32(Emulator* emu, ModRM* modrm)
{
    uint32_t value = get_rm32(emu, modrm);
    set_rm32(emu, modrm, value + 1);
}

static void code_ff(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    switch (modrm.opecode)
    {
    case 0:
        inc_rm32(emu, &modrm);
        break;
    default:
        printf("not implemented: ff /%d\n", modrm.opecode);
        exit(1);
    }
}

static void call_rel32(Emulator* emu)
{
    int32_t diff = get_sign_code32(emu, 1);
    push32(emu, emu->eip + 5);
    emu->eip += (diff + 5);
}

/**
 * スタックのトップには前に実行されたcallによってプッシュされたeipの番地がセットされている
 */
static void ret(Emulator* emu)
{
    emu->eip = pop32(emu);
}

/**
 * 呼び出し元のスタックポインタをespにセットし、呼び出し元の呼び出し元のespをebpにセットする
 * mov esp, ebp -> pop ebp をまとめて実行する
 */
static void leave(Emulator* emu)
{
    uint32_t ebp = get_register32(emu, EBP);
    set_register32(emu, ESP, ebp);
    set_register32(emu, ebp, pop32(emu));
    emu->eip += 1;
}

static void short_jump(Emulator* emu)
{
    int8_t diff = get_sign_code8(emu, 1);
    emu->eip += (diff + 2);
}

static void near_jump(Emulator* emu)
{
    int32_t diff = get_sign_code32(emu, 1);
    emu->eip += (diff + 5);
}

static void cmp_al_imm8(Emulator* emu)
{
    uint8_t value = get_code8(emu, 1);
    uint8_t al = get_register8(emu, AL);
    uint64_t result = (uint64_t) al - (uint64_t) value;
    update_eflags_sub(emu, al, value, result);
    emu->eip += 2;
}

static void cmp_eax_imm32(Emulator* emu)
{
    uint32_t value = get_code32(emu, 1);
    uint32_t eax = get_register32(emu, EAX);
    uint64_t result = (uint64_t)eax - (uint64_t)value;
    update_eflags_sub(emu, eax, value, result);
    emu->eip += 5;
}

/**
 * opecode = 0x3b
 */
static void cmp_r32_rm32(Emulator* emu)
{
    emu->eip += 1;
    ModRM modrm;
    parse_modrm(emu, &modrm);
    uint32_t r32 = get_r32(emu, &modrm);
    uint32_t rm32 = get_rm32(emu, &modrm);
    uint64_t result = (uint64_t)r32 - (uint64_t)rm32;
    update_eflags_sub(emu, r32, rm32, result);
}

#define define_jx(flag, is_flag) \
static void j ## flag(Emulator* emu) \
{ \
    int diff = is_flag(emu) ? get_sign_code8(emu, 1) : 0; \
    emu->eip += (diff + 2); \
} \
static void jn ## flag(Emulator* emu) \
{ \
    int diff = is_flag(emu) ? 0 : get_sign_code8(emu, 1); \
    emu->eip += (diff + 2); \
}

define_jx(c, is_carry)
define_jx(z, is_zero)
define_jx(s, is_sign)
define_jx(o, is_overflow)

#undef define_jx

static void jl(Emulator* emu)
{
    int diff = (is_sign(emu) != is_overflow(emu))
                ? get_sign_code8(emu, 1) : 0;
    emu->eip += (diff + 2);
}

static void jle(Emulator* emu)
{
    int diff = (is_zero(emu) || (is_sign(emu) != is_overflow(emu)))
                ? get_sign_code8(emu, 1) : 0;
    emu->eip += (diff + 2);
}

static void jg(Emulator* emu)
{
    int diff = !is_zero(emu) && (is_sign(emu) == is_overflow(emu))
               ? get_sign_code8(emu, 1) : 0;
    emu->eip += (diff + 2); 
}

void init_instructions(void)
{
    int i;
    memset(instructions, 0, sizeof(instructions));
    instructions[0x01] = add_rm32_r32;

    instructions[0x3B] = cmp_r32_rm32;
    instructions[0x3C] = cmp_al_imm8;
    instructions[0x3D] = cmp_eax_imm32;
    
    for (i = 0; i < 8; i++) {
        instructions[0x40 + i] = inc_r32;
    }

    for (i = 0; i < 8; i++) {
        instructions[0x50 + i] = push_r32;
    }

    for (i = 0; i < 8; i++) {
        instructions[0x58 + i] = pop_r32;
    }

    instructions[0x68] = push_imm32;
    instructions[0x6a] = push_imm8;

    instructions[0x70] = jo;
    instructions[0x71] = jno;
    instructions[0x72] = jc;
    instructions[0x73] = jnc;
    instructions[0x74] = jz;
    instructions[0x75] = jnz;
    instructions[0x78] = js;
    instructions[0x79] = jns;
    instructions[0x7c] = jl;
    instructions[0x7e] = jle;
    instructions[0x7f] = jg;

    instructions[0x83] = code_83;
    instructions[0x88] = mov_rm8_r8;
    instructions[0x89] = mov_rm32_r32;
    instructions[0x8A] = mov_r8_rm8;
    instructions[0x8B] = mov_r32_rm32;

    for (i = 0; i < 8; i++) {
        instructions[0xB0 + i] = mov_r8_imm8;
    }

    for (i = 0; i < 8; i++) {
        instructions[0xb8 + i] = mov_r32_imm32;
    }

    instructions[0xc3] = ret;
    instructions[0xc7] = mov_rm32_imm32;
    instructions[0xc9] = leave;

    instructions[0xe8] = call_rel32;
    instructions[0xe9] = near_jump;
    instructions[0xeb] = short_jump;
    instructions[0xec] = in_al_dx;
    instructions[0xee] = out_dx_al;
    instructions[0xff] = code_ff;

}

