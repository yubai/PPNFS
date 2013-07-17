/**
 * 一个变量可以同时被说明为const和volatile。
 * const修饰符的含义是变量的值不能被使用了const修饰符的那段代码修改，但这并不意味着它不能被这段代码以外的其它手段修改。
 * 例如，通过一个volatile const指针t来存取timer结构。
 * 函数time_addition()本身并不修改t->value的值，因此t->value被说明为const。
 * 不过，计算机的硬件会修改这个值，因此t->value又被说明为volatile。
 * 如果同时用const和volatile来说明一个变量，那么这两个修饰符随便哪个在先都行。
 */
#include <stdio.h>

int main(int argc, char *argv[])
{
    const volatile int val = 10;
    printf ("%d\n", val);
    return 0;
}

