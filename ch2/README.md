# 2.1
アセンブリ言語では、ほとんどの場合にレジスタを指定できる。
一方で、Ｃ言語からレジスタを直接に扱うことはできない。Ｃ言語では「変数」というレベルで抽象化されており、Ｃコンパイラが必要に応じてレジスタを走査するアセンブリ言語命令を出力する。

```
int i, sum = 0;
for (i = 0; i < 10; i++) {
    sum += 1;
}
```

# 2.5
```
void func(void)
{
    int val;
    int *ptr = &val;
    *ptr = 41;
}
```
↑のC言語のソースをアセンブラに変換したら次の通り
```
push ebp
mov ebp,esp
sub esp,byte +0x10   // 1
lea eax,[ebp-0x8]    // 2
mov [ebp-0x4],eax    // 3
mov eax,[ebp-0x4]    // 4
mov dword [eax],0x29 // 5
leave
ret
```
1
スタックポインタから即値を減算する-->スタック上に16byte分の領域を確保。

2
src([ebp-0x8])オペランドのアドレスをdst(eax)にロードする。
ここでは、val変数の番地をeaxレジスタに書き込んでいる。

3
eaxレジスタの値(=val変数のアドレス)をptr変数に書き込んでいる

4
ptr変数のメモリ番地をeaxレジスタに書き込んでいる

5
eaxが示すメモリ番地から4byteの領域に即値(0x29->41)を書き込んでいる


## 参考
- アセンブリ：https://jitera.com/ja/insights/38306

# memo
- make の仕組み 
  - https://qiita.com/hotoku/items/6e50c9f8864e98468ac7
  - https://zenn.dev/keitean/articles/aaef913b433677