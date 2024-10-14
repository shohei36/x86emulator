# 3.1
- BIOS
  - BIOSは特別で、不揮発性の記憶装置に書き込まれているが、CPUからは普通のメモリのように見える
  - BIOSには、HDDからOSを探し出してメモリ上に読み出し、そこへジャンプするようなプログラムがあらかじめ書き込まれている
- アドレス空間
  - CPUが論理アドレス空間と物理アドレス空間の対応表を持つ
  - OSがアドレス変換表を管理する役割をになっており、変換表を作成してCPUに渡す
- セグメンテーションとページング
  - セグメンテーション -->? """物理メモリ"""をいくつかのセグメントに分け、1つのプログラムを1つのセグメントに格納する。プログラムから見える0番地は、セグメントの先頭を表す
  - ページング --> """論理アドレス空間"""を小さな同じ大きさのページに分け、ページ単位で物理メモリを割り当てる
  - セグメンテーションとページンぐを持たないCPUでは、プログラムをメモリに配置する際に機械語を書き換えてラベルの番地を修正するなど工夫が必要

# 3.2
エミュレータのorg対応
プログラムが置かれたmemory番地の先頭、eipの初期値、orgの値の3つが一致して正しくプログラムが動作する

# 3.3
- CPUの実行過程
  - フェッチ ... CPUがメインメモリから命令を読み込む
  - デコード ... 命令を解釈する
  - 実行 ... 実行
- 汎用レジスタ
  - 命令によっては、計算に使うレジスタが固定されている
    - 例えばesp。スタックの先頭をさすために使われる。call/ret命令、push/pop命令で暗黙に読み書きされる
  - 汎用レジスタはすべて32ビット幅だが、別名がついていることがある
    - e[abcd]x 32
      - [abcd]x 下位16bit
        - [abcd]h 上位8bit
        - [abcd]l 下位8bit
- 特殊レジスタ
  - eip
    - 命令ポインタ
    - 現在実行中の命令の番地を記憶する
  - eflags
    - 演算結果やCPUの状態などを表す 
- アドレスバスとデータバス
  - アドレスバスは、CPUがどの装置の何番地を読み書きするかを指示する
  - データバスには、実際のデータが流れる
- 機械語命令の構造
  - [プレフィックス(0~4)|オペコード(1,2,3)|ModR/M(0,1)|SIB(0,1)|ディスプレースメント(0,1,2,4)|イミディエイト(0,1,2,4)]
  - プレフィックス
    - 機械語命令の先頭に付加される1バイト（または複数バイト）**の特別なコードで、命令の挙動や動作モードを変更するために使用
  - オペコード
    - 命令の種類を表す
  - ModR/M ... オペランド(命令の操作対象)を指定するために使われるバイト
    - Mod(2) REG(3) RM(3) 
    - Mod ... オペランドがレジスタを指すか、メモリアドレスを指すかを指定する
    - Reg ... 操作対象のレジスタを指定する
    - R/M ... 使用するレジスタやメモリアドレッシングモードを指定する
  - SIB
  - ディスプレースメント
    - レジスタと組み合わせてメモリ番地を指定する
    - ModR/Mの値によって要不要とバイト数が決まる
  - イミディエイト

inc [ebp - 4]
機械語にすると、
0xff 0x45 0xfc

まず先頭1バイトをフェッチする。
0xff は プレフィックスとして有効な値ではないので、オペコードであることがわかる。
0xff は inc or dec のどちらか。
オペコード 0xff の仕様から、ModR/Mバイトが存在して、その中のREGをオペコードの拡張として使うことが決まる。

ModR/Mバイトが存在することが決まっているので、次の1バイトをフェッチする。
0x45 を2進数に直すと 01000101 となる。
Mod -> 01
REG -> 000
RM  -> 101
REGは 000 なので、incであることがわかる。
Mod=01,RM=101 なので、アドレッシングモードが[ebp]+disp8 であることがわかる。
(1byteのディスプレースメントが存在する。SIBは存在しない)

1バイトのディスプレースメントが存在することがわかったので、次の1バイトをフェッチする。
0xfc はディスプレースメント。符号付き整数なので、これは-4に対応する。

# 3.5
- eipの値を変更すればjmpできるが、add や mov では epi をオペランドに指定できない
- push
  - espを4だけ減少させる
  - espがさすメモリ領域にオペランドを書き込む
- pop
  - espが指すメモリ領域から値を読み取ってオペランドに書き込む
  - espを4だけ増加させる
- call
  - call メモリ番地 のように書くと、現在のeipの値をスタックにプッシュし、さらにジャンプ先のメモリ番地をeipに書き込む
  - 挙動としては、push eip --> jmp メモリ番地 と同じだが、pushではeipをオペランドに指定できない
- ret
  - ret はスタックから4バイトの値をポップし、その値をeipにセットする
  - pop eip と同じだが、popではeipをオペランドに指定できない。
  - プログラムが正しければ、スタックの先頭にはcall命令によりプッシュされた戻り先のメモリ番地が書いてあるはず

```
int my_add(int a, int b)
{
  return a + b;
}

void func(void)
{
  int v = my_add(3, 5);
}
```
↑のC言語のコードをコンパイルすると以下の通り
```
00000000  55                       push ebp           -- my_add begin
00000001  89E5                     mov ebp,esp
00000003  8B5508                   mov edx,[ebp+0x8]
00000006  8B450C                   mov eax,[ebp+0xc]
00000009  01D0                     add eax,edx
0000000B  5D                       pop ebp
0000000C  C3                       ret                -- my_add end
0000000D  55                       push ebp           -- func begin
0000000E  89E5                     mov ebp,esp
00000010  83EC10                   sub esp,byte +0x10
00000013  6A05                     push byte +0x5
00000015  6A03                     push byte +0x3
00000017  EBE4FFFFFF               call 0x100000000 
0000001C  83C408                   add esp,byte +0x8                   
0000001F  8945FC                   mov [ebp-0x4],eax
00000022  C9                       leave
00000023  C3                       ret                -- func end
```

# 3.6
- 戻り値に使われるレジスタはeax。関数からretで戻ってきたときにeaxに入っている値が戻り値というルール
- 

# 3.7
- crt0 は、Cランタイムの開始コード

```
int add(int a, int b)
{
    return a + b;
}

int main(void)
{
    return add(2, 5);
}

```
↓アセンブラ
```
C:\Users\tkmr0\github\x86emulator\tolset_p86\exec-arg-test>ndisasm -b 32 test.bin
00000000  E812000000        call dword 0x17
00000005  E9F683FFFF        jmp dword 0xffff8400
0000000A  55                push ebp
0000000B  89E5              mov ebp,esp
0000000D  8B5508            mov edx,[ebp+0x8]
00000010  8B450C            mov eax,[ebp+0xc]
00000013  01D0              add eax,edx
00000015  5D                pop ebp
00000016  C3                ret
00000017  55                push ebp
00000018  89E5              mov ebp,esp
0000001A  6A05              push byte +0x5
0000001C  6A02              push byte +0x2
0000001E  E8E7FFFFFF        call dword 0xa
00000023  83C408            add esp,byte +0x8
00000026  C9                leave
00000027  C3                ret
```
実行結果
```
C:\Users\tkmr0\github\x86emulator\ch3>main test.bin
EIP = 7C00, ESP = 7C00, EBP = 0, Code = E8, StackHead = 12E8
EIP = 7C17, ESP = 7BFC, EBP = 0, Code = 55, StackHead = 7C05
EIP = 7C18, ESP = 7BF8, EBP = 0, Code = 89, StackHead = 0
EIP = 7C1A, ESP = 7BF8, EBP = 7BF8, Code = 6A, StackHead = 0
EIP = 7C1C, ESP = 7BF4, EBP = 7BF8, Code = 6A, StackHead = 5
EIP = 7C1E, ESP = 7BF0, EBP = 7BF8, Code = E8, StackHead = 2
EIP = 7C0A, ESP = 7BEC, EBP = 7BF8, Code = 55, StackHead = 7C23
EIP = 7C0B, ESP = 7BE8, EBP = 7BF8, Code = 89, StackHead = 7BF8
EIP = 7C0D, ESP = 7BE8, EBP = 7BE8, Code = 8B, StackHead = 7BF8
EIP = 7C10, ESP = 7BE8, EBP = 7BE8, Code = 8B, StackHead = 7BF8
EIP = 7C13, ESP = 7BE8, EBP = 7BE8, Code = 01, StackHead = 7BF8
EIP = 7C15, ESP = 7BE8, EBP = 7BE8, Code = 5D, StackHead = 7BF8
EIP = 7C16, ESP = 7BEC, EBP = 7BF8, Code = C3, StackHead = 7C23
EIP = 7C23, ESP = 7BF0, EBP = 7BF8, Code = 83, StackHead = 2
EIP = 7C26, ESP = 7BF8, EBP = 7BF8, Code = C9, StackHead = 0
EIP = 7C27, ESP = 7BFC, EBP = 0, Code = C3, StackHead = 7C05
EIP = 7C05, ESP = 7C00, EBP = 0, Code = E9, StackHead = 12E8


end of program.

EAX = 00000007
,ECX = 00000000
,EDX = 00000002
,EBX = 00000000
,ESP = 00007c00
,EBP = 00000000
,ESI = 00000000
,EDI = 00000000
,EIP = 00000000
```

ローカル変数アリの場合を確認してみる
```
int add(int a, int b)
{
    return a + b;
}

int main(void)
{
    int val;
    val = add(2, 5);
    return val;
}
```
アセンブラ
```
C:\Users\tkmr0\github\x86emulator\tolset_p86\exec-arg-test>ndisasm -b 32 test.bin
00000000  E812000000        call dword 0x17
00000005  E9F683FFFF        jmp dword 0xffff8400
0000000A  55                push ebp
0000000B  89E5              mov ebp,esp
0000000D  8B5508            mov edx,[ebp+0x8]
00000010  8B450C            mov eax,[ebp+0xc]
00000013  01D0              add eax,edx
00000015  5D                pop ebp
00000016  C3                ret
00000017  55                push ebp
00000018  89E5              mov ebp,esp
0000001A  83EC10            sub esp,byte +0x10
0000001D  6A05              push byte +0x5
0000001F  6A02              push byte +0x2
00000021  E8E4FFFFFF        call dword 0xa
00000026  83C408            add esp,byte +0x8
00000029  8945FC            mov [ebp-0x4],eax
0000002C  8B45FC            mov eax,[ebp-0x4]
0000002F  C9                leave
00000030  C3                ret
```
実行結果
```
C:\Users\tkmr0\github\x86emulator\ch3>main test.bin
EIP = 7C00, ESP = 7C00, EBP = 0, Code = E8, StackHead = 12E8
EIP = 7C17, ESP = 7BFC, EBP = 0, Code = 55, StackHead = 7C05
EIP = 7C18, ESP = 7BF8, EBP = 0, Code = 89, StackHead = 0
EIP = 7C1A, ESP = 7BF8, EBP = 7BF8, Code = 83, StackHead = 0
EIP = 7C1D, ESP = 7BE8, EBP = 7BF8, Code = 6A, StackHead = 0
EIP = 7C1F, ESP = 7BE4, EBP = 7BF8, Code = 6A, StackHead = 5
EIP = 7C21, ESP = 7BE0, EBP = 7BF8, Code = E8, StackHead = 2
EIP = 7C0A, ESP = 7BDC, EBP = 7BF8, Code = 55, StackHead = 7C26
EIP = 7C0B, ESP = 7BD8, EBP = 7BF8, Code = 89, StackHead = 7BF8
EIP = 7C0D, ESP = 7BD8, EBP = 7BD8, Code = 8B, StackHead = 7BF8
EIP = 7C10, ESP = 7BD8, EBP = 7BD8, Code = 8B, StackHead = 7BF8
EIP = 7C13, ESP = 7BD8, EBP = 7BD8, Code = 01, StackHead = 7BF8
EIP = 7C15, ESP = 7BD8, EBP = 7BD8, Code = 5D, StackHead = 7BF8
EIP = 7C16, ESP = 7BDC, EBP = 7BF8, Code = C3, StackHead = 7C26
EIP = 7C26, ESP = 7BE0, EBP = 7BF8, Code = 83, StackHead = 2
EIP = 7C29, ESP = 7BE8, EBP = 7BF8, Code = 89, StackHead = 0
EIP = 7C2C, ESP = 7BE8, EBP = 7BF8, Code = 8B, StackHead = 0
EIP = 7C2F, ESP = 7BE8, EBP = 7BF8, Code = C9, StackHead = 0
EIP = 7C30, ESP = 7BFC, EBP = 0, Code = C3, StackHead = 7C05
EIP = 7C05, ESP = 7C00, EBP = 0, Code = E9, StackHead = 12E8


end of program.

EAX = 00000007
,ECX = 00000000
,EDX = 00000002
,EBX = 00000000
,ESP = 00007c00
,EBP = 00000000
,ESI = 00000000
,EDI = 00000000
,EIP = 00000000
```

# 3.9
- subを使えば、条件分岐命令の条件をすべてかけてしまう
- cmp命令はsubと異なり、引き算した結果をオペランドにセットしない点が異なる

# 3.10

c
```
int abs(int i)
{
    if (i >= 0) {
        return i;
    } else {
        return -i;
    }
}

int main(void)
{
    return abs(-3);
}
```

アセンブラ
```
C:\Users\tkmr0\github\x86emulator\tolset_p86\exec-if-test>ndisasm -b 32 test.bin
00000000  E81A000000        call dword 0x1f
00000005  E9F683FFFF        jmp dword 0xffff8400
0000000A  55                push ebp
0000000B  89E5              mov ebp,esp
0000000D  837D0800          cmp dword [ebp+0x8],byte +0x0
00000011  7805              js 0x18
00000013  8B4508            mov eax,[ebp+0x8]
00000016  EB05              jmp short 0x1d
00000018  8B4508            mov eax,[ebp+0x8]
0000001B  F7D8              neg eax
0000001D  5D                pop ebp
0000001E  C3                ret
0000001F  55                push ebp
00000020  89E5              mov ebp,esp
00000022  B803000000        mov eax,0x3
00000027  5D                pop ebp
00000028  C3                ret
```

実行結果
--> あれ、abs関数が実行されていない、、？
```
C:\Users\tkmr0\github\x86emulator\ch3>main.exe test.bin
EIP = 7C00, ESP = 7C00, EBP = 0, Code = E8, StackHead = 1AE8
EIP = 7C1F, ESP = 7BFC, EBP = 0, Code = 55, StackHead = 7C05
EIP = 7C20, ESP = 7BF8, EBP = 0, Code = 89, StackHead = 0
EIP = 7C22, ESP = 7BF8, EBP = 7BF8, Code = B8, StackHead = 0
EIP = 7C27, ESP = 7BF8, EBP = 7BF8, Code = 5D, StackHead = 0
EIP = 7C28, ESP = 7BFC, EBP = 0, Code = C3, StackHead = 7C05
EIP = 7C05, ESP = 7C00, EBP = 0, Code = E9, StackHead = 1AE8


end of program.

EAX = 00000003
,ECX = 00000000
,EDX = 00000000
,EBX = 00000000
,ESP = 00007c00
,EBP = 00000000
,ESI = 00000000
,EDI = 00000000
,EIP = 00000000
```

# 3.11
c
```
int sum(int a, int b)
{
  int sum;
  sum = 0;
  while (a <= b) {
    sum += a;
    a++;
  }
  return sum;
}

int main(void)
{
    return sum(1, 10);
}
```
アセンブラ
```
C:\Users\tkmr0\github\x86emulator\tolset_p86\exec-while-stmt>ndisasm -b 32 test.bin
00000000  E82A000000        call dword 0x2f
00000005  E9F683FFFF        jmp dword 0xffff8400
0000000A  55                push ebp
0000000B  89E5              mov ebp,esp
0000000D  83EC10            sub esp,byte +0x10
00000010  C745FC00000000    mov dword [ebp-0x4],0x0
00000017  EB09              jmp short 0x22
00000019  8B4508            mov eax,[ebp+0x8]
0000001C  0145FC            add [ebp-0x4],eax
0000001F  FF4508            inc dword [ebp+0x8]
00000022  8B4508            mov eax,[ebp+0x8]
00000025  3B450C            cmp eax,[ebp+0xc]
00000028  7EEF              jng 0x19
0000002A  8B45FC            mov eax,[ebp-0x4]
0000002D  C9                leave
0000002E  C3                ret
0000002F  55                push ebp
00000030  89E5              mov ebp,esp
00000032  6A0A              push byte +0xa
00000034  6A01              push byte +0x1
00000036  E8CFFFFFFF        call dword 0xa
0000003B  83C408            add esp,byte +0x8
0000003E  C9                leave
0000003F  C3                ret
```

jg を実装してみる
実行したいプログラムは以下
```
int sum(int a, int b)
{
    int sum = 0;
    goto cond;
loop:
    sum += a;
    a++;
cond:
    if (a <= b)
    {
        goto loop;
    }
    return sum;
}
```

アセンブル結果
```
C:\Users\tkmr0\github\x86emulator\tolset_p86\exec-while-stmt>ndisasm -b 32 test2.bin
00000000  E82C000000        call dword 0x31
00000005  E9F683FFFF        jmp dword 0xffff8400
0000000A  55                push ebp
0000000B  89E5              mov ebp,esp
0000000D  83EC10            sub esp,byte +0x10
00000010  C745FC00000000    mov dword [ebp-0x4],0x0
00000017  EB09              jmp short 0x22
00000019  8B4508            mov eax,[ebp+0x8]
0000001C  0145FC            add [ebp-0x4],eax
0000001F  FF4508            inc dword [ebp+0x8]
00000022  8B4508            mov eax,[ebp+0x8]
00000025  3B450C            cmp eax,[ebp+0xc]
00000028  7F02              jg 0x2c
0000002A  EBED              jmp short 0x19
0000002C  8B45FC            mov eax,[ebp-0x4]
0000002F  C9                leave
00000030  C3                ret
00000031  55                push ebp
00000032  89E5              mov ebp,esp
00000034  6A0A              push byte +0xa
00000036  6A01              push byte +0x1
00000038  E8CDFFFFFF        call dword 0xa
0000003D  83C408            add esp,byte +0x8
00000040  C9                leave
00000041  C3                ret
```
jg こんなんでいいのかな
```
static void jg(Emulator* emu)
{
    int diff = !is_zero(emu) && (is_sign(emu) == is_overflow(emu))
               ? get_sign_code8(emu, 1) : 0;
    emu->eip += (diff + 2); 
}
```

# 3.12

assemble
```
BITS 33
    org 0x7c00
start:
    mov edx, 0x03f8
mainloop:
    mov al, '>'    ; プロンプトを表示
    out dx, al
input:
    in al, dx       ; 1文字入力
    cmp al, 'h'
    je puthello     ; hならhelloを表示
    cmp al, 'w'
    je putworld     ; wならworldを表示
    cmp al, 'q'
    je fin          ; qなら終了
    jmp input       ; それ以外なら再入力
puthello:
    mov esi, msghello
    call puts
    jmp mainloop
putworld:
    mov esi, msgworld
    call puts
    jmp mainloop
fin:
    jmp 0

; esiに設定された文字列を表示するサブルーチン
puts:
    mov al, [esi]
    inc esi
    cmp al, 0
    je putsend
    out dx, al
    jmp puts
putsend:
    ret

msghello:
    db "hello", 0x0d, 0x0a, 0
msgworld:
    db "world", 0x0d, 0x0a, 0
```

dis assemble
```
C:\Users\tkmr0\github\x86emulator\ch3>ndisasm -b 32 select.bin
00000000  BAF8030000        mov edx,0x3f8
00000005  B03E              mov al,0x3e
00000007  EE                out dx,al
00000008  EC                in al,dx
00000009  3C68              cmp al,0x68
0000000B  740A              jz 0x17
0000000D  3C77              cmp al,0x77
0000000F  7412              jz 0x23
00000011  3C71              cmp al,0x71
00000013  741A              jz 0x2f
00000015  EBF1              jmp short 0x8
00000017  BE3F7C0000        mov esi,0x7c3f
0000001C  E813000000        call dword 0x34
00000021  EBE2              jmp short 0x5
00000023  BE477C0000        mov esi,0x7c47
00000028  E807000000        call dword 0x34
0000002D  EBD6              jmp short 0x5
0000002F  E9CC83FFFF        jmp dword 0xffff8400
00000034  8A06              mov al,[esi]
00000036  46                inc esi
00000037  3C00              cmp al,0x0
00000039  7403              jz 0x3e
0000003B  EE                out dx,al
0000003C  EBF6              jmp short 0x34
0000003E  C3                ret
0000003F  68656C6C6F        push dword 0x6f6c6c65
00000044  0D0A00776F        or eax,0x6f77000a
00000049  726C              jc 0xb7
0000004B  64                fs
0000004C  0D                db 0x0d
0000004D  0A00              or al,[eax]
```

汎用レジスタの部位
https://qiita.com/Ll_e_ki/items/fd098cb6d1b6390a76e7#%E6%B1%8E%E7%94%A8%E3%83%AC%E3%82%B8%E3%82%B9%E3%82%BF%E3%81%AE%E9%83%A8%E4%BD%8D
```
|        EAX        | 32bit
          |   AX    | 16bit
          | AH | AL | 8bit
```