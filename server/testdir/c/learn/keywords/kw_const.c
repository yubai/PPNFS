#include <stdio.h>

const int intMax = 100;
int array[intMax];  // 全局变量，报错

int main(int argc, char *argv[])
{
    const int SIZE = 100;
    int IntArray[SIZE];  // 局部变量，gcc4.4.3编译没有发生错误。
    
    return 0;
}

