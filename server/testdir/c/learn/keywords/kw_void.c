/**
 * 对于add和del函数，直接编译可能不提示warning。需要加上-Wall选项才会提示。
 */

#include <stdio.h>


/**
 * 在C语言中，凡不加返回值类型限定的函数，就会被编译器作为返回整型值处理。
 * 但是许多程序员却误以为其为void类型。
 */
add ( int a, int b ) 
{ 
    return a + b; 
}

/**
 * 下面这个函数默认是返回int型数据，我没有写return语句。结果编译还是通过。
 * 然后main函数中打印出来的值是10，返回的是eax的值。
 */
del(void)
{
}

int main(int argc, char *argv[])
{
    printf ( "2 + 3 = %d\n", add ( 2, 3) );
    printf ("%d\n", del());  // output: 10 返回eax

    void *pvoid; 
    pvoid++; // gcc编译通过，但不推荐这样使用。
    (char *)pvoid++; //推荐--ANSI：正确；GNU：正确

    // void a; // error: variable or field ‘a’ declared void
    return 0;
}
