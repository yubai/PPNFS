/**
 * extern就是告诉编译器，link的时候去symbol table里面找而已。
 * 所以，可以在本文件中定义，同时又用extern声明一个变量。
 */

#include <stdio.h>

int i = 10;

extern int i;  // OK

char a[6];

//extern char *a;  // 编译不通过，error: conflicting types for ‘a’
// 原因在于，指向类型T的指针并不等价于类型T的数组。
// extern char *a声明的是一个指针变量而不是字符数组，因此与实际的定义不同，从而造成运行时非法访问。
extern char a[];  // OK


void foo(void)  // 参数void可以去掉。如果extern中写了void，则有warning。
// warning: prototype for ‘foo’ follows non-prototype definition
{
    
}

extern void foo(void); // 参数void可以去掉。

int j = 1;
//extern double j; // error: conflicting types for ‘j’

int main(int argc, char *argv[])
{
    printf ("%d\n", i);
    return 0;
}

