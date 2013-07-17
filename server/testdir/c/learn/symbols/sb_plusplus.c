#include <stdio.h>

int main(int argc, char *argv[])
{
    int i = 0;
    int j = 0;

    int a = 1;
    int b = 2;

    int sum;
    
    if (++i > 0 || ++j > 0)  // 先算++i，满足大于0后，++j不再计算。
    {
        printf ("%d %d\n", i, j);  // output: 1, 0
    }

    i = 3;
    sum = (++i)+(++i)+(++i);
    printf ("%d\n", sum);  // output: 16

//    sum = a+++++b;  // error: lvalue required as increment operand
    sum = a++ + ++b;  // OK.
    printf ("%d\n", sum);
    return 0;
}

