#include <stdio.h>
#include <stdlib.h>

struct student
{
    
}stu; 

typedef struct st_type
{
    int i;
    int a[];
}type_a;

int main(int argc, char *argv[])
{
    //stu s; // stu是一个变量，不是类型，不能用于定义变量。
    printf ("%d\n", sizeof(stu));  // output: 0

    printf ("%d\n", sizeof(type_a));  // output: 4

    type_a *p = (type_a*)malloc(sizeof(type_a)+100*sizeof(int));
    printf ("%d\n", sizeof(p));  // output: 4

    
    return 0;
}
