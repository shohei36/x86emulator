#include <stdio.h>

#define HELLO(sb) \
static void hello_ ## sb(char* target) \
{ \
    printf("hello, %s.\n", target); \
}

HELLO(takashi)

# undef HELLO

int main()
{
    hello_takashi("takashi");
}
