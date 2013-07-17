/*
x + y =(x & y ) + (x | y)
x | y = (x & y) + (x ^ y)
*/
#include <stdio.h>

int f(int x, int y)
{
    return (x&y) + ((x^y) >> 1);
}

int main(int argc, char *argv[])
{
    int a, b;
    while (1) {
        scanf("%d %d", &a, &b);
        printf ("%d %d\n", f(a, b), (a + b) / 2);
    }
    return 0;
}

