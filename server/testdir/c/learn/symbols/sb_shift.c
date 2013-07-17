#include <stdio.h>

int main(int argc, char *argv[])
{
    printf ("%d\n", 0x01 << 2 + 3);  // output: 32。 “+”号的优先级比移位运算符的优先级高

    printf("%d\n", 0x01 << 2 + 30);  // output: 0. warning: left shift count >= width of type

    printf("%d\n", 0x01 << 2 - 3);   // output: 0. warning: left shift count is negative


    return 0;
}

