#include <stdio.h>

enum enum_type_name
{  // 元素之间默认用,隔开。 
    ENUM_CONST_1,  // 默认从0开始
    ENUM_CONST_2,
    ENUM_CONST_3,
    ENUM_CONST_4    
}enum_variable_name;

int main(int argc, char *argv[])
{
    enum_variable_name = ENUM_CONST_1;
    printf ("%d\n", enum_variable_name);  // output: 0

    printf ("%d\n", sizeof(enum_variable_name));  // output: 4
    return 0;
}

