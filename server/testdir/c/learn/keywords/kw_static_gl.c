/**
 *  使用命令gcc -g ***.c -o ***生成可被gdb使用的可执行文件。
 *  结果是：每次循环之后，i的值加1，j的值维持1不变。
 */

#include <stdio.h>

static int j;

void fun1(void)
{
    static int i = 0;
    i ++;
}

void fun2(void)
{
    j = 0;
    j++;
}

int main()
{
    int k;
    for(k=0; k<10; k++)
    {
        fun1();
        fun2();
    }

    return 0;
}

