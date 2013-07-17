#include <stdio.h>

//这是一条合法的\
单行注释
/\
/这是一条合法的单行注释
#def\
ine MAC\
RO 这是一条合法的\
宏定义
cha\
r* s="这是一个合法的\\
n 字符串";


#define THIS_IS_A_LONG_VALUE 120

int main(int argc, char *argv[])
{
    printf ("Hello, world!    \
            %d \n", THIS_IS_A_\
LONG_VALUE);  //  不能有空格
    
    return 0;
}
