#include <stdio.h>

typedef struct student
{
//code
}Stu_st,*Stu_pst;//命名规则请参考本章前面部分


// typedef int int32;
//unsigned int32 i = 10;  // error， 不支持


//typedef static int M_INT32;
//unsigned M_INT32 i = 10;


typedef char* pchar;
pchar p1,p2;  // OK

int main(int argc, char *argv[])
{
    Stu_st stu1, stu2;
    const Stu_pst p1 = &stu1;  // 标明p1是个常量指针，一旦初始化，不可再改变。
    // p1 = &stu2;    // error: assignment of read-only variable ‘p1’

    Stu_pst const p2 = &stu1;  // 标明p2是个常量指针，一旦初始化，不可再改变。
    //p2 = &stu2;  //  error: assignment of read-only variable ‘p2’
    return 0;
}

