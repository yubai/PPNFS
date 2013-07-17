#include <stdio.h>

int foo()
{
    return;
}

int main(int argc, char *argv[])
{
    printf ("Hello, world!\n");
    printf ("%d\n", foo());  // output: 14 默认返回eax的值。
    return;  // It's OK!
}

