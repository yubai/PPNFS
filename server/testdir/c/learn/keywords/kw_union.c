#include <stdio.h>

union
{
int i;
char a[2];
}*p, u;


int main(int argc, char *argv[])
{
    p =&u;
    p->a[0] = 0x39;
    p->a[1] = 0x38;
    printf ("%d\n", p->i);  // output: 14393
    // 1.小端存储，字数据的低字节则存放在低地址中。
    // 2.使用了char数组，从低地址开始存储。i的值为0x00003839=14393。
    printf ("%d\n", sizeof(u));  // output: 4


    int a[5]={1,2,3,4,5};
    int *ptr1=(int *)(&a+1);  // ptr1指向数组a之后（5之后）的第一个元素。
    // 因为a的类型是int[5], 加上&符，取地址就变成int[5]*类型了。所以，加1后会跨越整个int[5]的数组。
//    unsigned char* p = (unsigned char*) a + 4;
//    printf ("%u %u %u %u %u %u\n", ptr1, &a[0], &a[1], a, &a, p);
    int *ptr2=(int *)((int)a+1);
    printf("%x,%x\n",ptr1[-1],*ptr2);  // ptr1[-1]指向数组中的5。output: 5,2000000
    // ptr1[-1]等同于ptr1 - 1。ptr1是int*类型，因此向前移动一个int类型。&a+1 

    return 0;
}

