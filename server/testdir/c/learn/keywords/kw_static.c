/**
 *  static关键字的作用域是从定义之处开始,到文件结尾处结束。
 *  在定义之处前面的那些代码行也不能使用它。想要使用就得在前面再加 extern ***。
 */

#include <stdio.h>

static int static_var = 1;


void count(int arg)
{
    printf ("%d\n", arg + static_var);
}

/**
 * if static_var is defined here, then it will result in the compile error show as below:
 * kw_static.c: In function ‘count’:
 * kw_static.c:5: error: ‘static_var’ undeclared (first use in this function)
 * kw_static.c:5: error: (Each undeclared identifier is reported only once
 * kw_static.c:5: error: for each function it appears in.)
 */
// static int static_var = 1;

int main(int argc, char *argv[])
{
    count(1);
    return 0;
}
