
int inc(int v)
{
    return v + 1;
}

/**
 * 関数ポインタ
 * int (int) --> 関数の型
 * * --> 関数ポインタ
 * ptr --> 変数名
 */
int (*ptr)(int) = &inc; 

typedef int func_t(int);
func_t *ptr2 = &inc;