/**
 * 此题看上去真的很简单，但是却鲜有人答对。答案是255。别惊讶，我们先分析分析。
 * for循环内，当i 的值为0 时，a[0]的值为-1。关键就是-1 在内存里面如何存储。
 * 我们知道在计算机系统中，数值一律用补码来表示（存储）。主要原因是使用补码，可以将符号位和其它位统一处理；同时，减法也可按加法来处理。另外，两个用补码表示的数相加时，如果最高位（符号位）有进位，则进位被舍弃。正数的补码与其原码一致；负数的补码：符号位为1，其余位为该数绝对值的原码按位取反，然后整个数加1。按照负数补码的规则，可以知道-1 的补码为0xff，-2 的补码为0xfe……当i 的值为127 时，a[127]的值为-128，而-128 是char 类型数据能表示的最小的负数。当i 继续增加，a[128]的值肯定不能是-129。因为这时候发生了溢出，-129 需要9 位才能存储下来，而char 类型数据只有8 位，所以最高位被丢弃。剩下的8 位是原来9 位补码的低8 位的值，即0x7f。
 * 当i 继续增加到255 的时候，-256 的补码的低8 位为0。然后当i 增加到256 时，-257 的补
码的低8 位全为1，即低八位的补码为0xff，如此又开始一轮新的循环……
 * 按照上面的分析，a[0]到a[254]里面的值都不为0，而a[255]的值为0。strlen 函数是计算字符串长度的，并不包含字符串最后的‘\0’。而判断一个字符串是否结束的标志就是看是否遇到‘\0’。如果遇到‘\0’，则认为本字符串结束。
 * 分析到这里，strlen(a)的值为255 应该完全能理解了。这个问题的关键就是要明白char类型默认情况下是有符号的，其表示的值的范围为[-128,127]，超出这个范围的值会产生溢出。另外还要清楚的就是负数的补码怎么表示。弄明白了这两点，这个问题其实就很简单了。
 */
   
#include <stdio.h>
#include <string.h>

int main() 
{ 
    char a[1000]; 
    int i; 
    for(i=0; i<1000; i++) 
    { 
        a[i] = -1-i; 
    }
    printf("%d\n",strlen(a));  // output: 255

    /*-----------------------------------------------*/
    int m = -20; 
    unsigned int n = 10;
    printf ("%d\n", m + n);  // output: -10

    /*-----------------------------------------------*/
    unsigned k ; 
    for (k=9;k>=0;k--) 
    { 
        printf("%u\n",k);  // output: 无限循环，因为k不会变为负数。
    } 
    return 0;
}