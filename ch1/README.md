# 1.3

機械語プログラムを逆アセンブルする。

逆アセンブル対象のバイナリファイルを生成する
```
> cd tolset_p86/casm-c-sample
> .\!cons_nt.bat
> gcc -Wl,--entry=func,--oformat=binary -nostdlib -fno-asynchronous-unwind-tables -o casm-c-sample.bin casm-c-sample.c
```

逆アセンブルを実行する。
-b 32 とすることで、x86アーキテクチャの中でも32ビットモードのプログラムであることを指定している。
```
>ndisasm -b 32 casm-c-sample.bin
00000000  55                push ebp
00000001  89E5              mov ebp,esp
00000003  83EC10            sub esp,byte +0x10
00000006  C745FC00000000    mov dword [ebp-0x4],0x0
0000000D  FF45FC            inc dword [ebp-0x4]
00000010  C9                leave
00000011  C3                ret
```

C言語プログラムをデバッグ情報付きでコンパイルし、出力されたオブジェクトファイルを見てみる。
objdumpが出力するアセンブリ言語はNASMの書式と少し異なるので注意。完全なNASM文法を見たい場合は、ndisasm を使う。
gcc の -c は、C言語ソースコードのコンパイルだけを行い、リンクをしない指定。
gcc の -g は、Ｃ言語の行と機械語の対応関係などのでデバッグ情報を出力結果に含める指定。
objdump の -d は、逆アセンブルモード。
objdump の -S は、逆アセンブル結果にＣ言語のソースコードを混ぜて表示。
objdump の -M intel は、逆アセンブル結果をintel表記で表示。(intel表記のほうがNASM文法に近い)
```
> gcc -c -g -o casm-c-sample.o casm-c-sample.c
> objdump -d -S -M intel casm-c-sample.o

casm-c-sample.o:     file format elf32-i386

Disassembly of section .text:

00000000 <func>:
void func(void) {
   0:   55                      push   ebp
   1:   89 e5                   mov    ebp,esp
   3:   83 ec 10                sub    esp,0x10
  int val = 0;
   6:   c7 45 fc 00 00 00 00    mov    DWORD PTR [ebp-0x4],0x0
  val++;
   d:   ff 45 fc                inc    DWORD PTR [ebp-0x4]
}
  10:   c9                      leave
  11:   c3                      ret
```

# 1.4
アセンブリ言語の１命令
```
オペコード  オペランド1, オペランド2
```

# 1.5
mov命令
```
mov 移動先, 移動元
```
移動元から移動先へデータを移動する。
一度に移動できるデータは 4,2,1byte。
移動元には変更を加えない（実質Copy）

```
mov ebp, esp
```
esp -> ebp に値をコピーする。
esp, ebp はCPU内臓のレジスタで、任意の32bitの値を記憶できる。
esp はスタックポインタ。スタックの最新の読み書き位置を保持する。
ここでは、その瞬間のスタックポインタを後で使うためにebpに保持させている。

```
mov dword [ebp-0x4],0x0
```
0x0 -> dword [ebp-0x4] に値をコピーする。
ebp-0x4 は、コピー先のメモリ番地を計算
[] は、メモリ番地を表す
dword は、領域の大きさを指定
ebp-0x4は、変数valの先頭位置となる。

# 1.6
inc命令
```
inc インクリメント対象
```
inc命令は、２つの機械語命令が１つのアセンブリ言語命令に対応する例外的な命令。

# 1.7
2進数4桁と16進数1桁がピッタリ対応する。

# 1.8
2の補数は、2進数で負の数を表す1つの方法。
2の補数は、1桁だけ大きい2のべき数(最上位ビットが1でほかのビットがすべて0の2進数)から元の整数を引けば求まる。
ex) 41の2の補数
```
  100000000 2のべき数
-  00101001 41
---------------------
   11010110 -41
```

2の補数表現のビット列を表示するプログラムを実行
```
> cd tolset_p86\casm-2compl-c
> make
make compl.exe
make[1]: Entering directory 'C:/Users/tkmr0/github/x86emulator/tolset_p86/casm-2compl-c'
../z_tools/mingw32-gcc/bin/gcc -Wall -c compl.c
../z_tools/mingw32-gcc/bin/gcc -o compl.exe compl.o
> compl.exe
signed = -41 (ffffffd7), unsigned = 215 (d7)
```

2の補数には、1で空きビットを埋めて拡張される(ゆえに、大量のf(16)がついている。)
符号なし整数の場合は、常に0で空きビットを埋めて拡張される。

2の補数を使うと、引き算を足し算だけで計算できる。こうすると、ＣＰＵに加算器しか搭載せず回路を単純化でき、コストを下げることができる。
```
  00000111 7
+ 11110001 -15
---------------
  11111000 
```

引き算を使わずに2の補数を求めることができる。
全ビットを反転してから1を足す。
```
~00001111(2) + 1 ~はビット反転を表す
=11110000(2) + 1
=111100001(2)
```