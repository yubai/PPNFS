/**
 * 不开辟用于交换数据的临时空间，如何完成字符串的逆序(在技术一轮面试中，有些面试官会这样问)
 */

#include <stdio.h>
#include <string.h>

void change(char* str) {
    for(int i=0,j=strlen(str)-1; i<j; i++, j--){
        str[i] ^= str[j];
        str[j] ^= str[i];
        str[i] ^= str[j];
//        str[i] ^= str[j] ^= str[i] ^= str[j]; // 这样不管用，不知道什么原因。
    }
}

int main(int argc, char* argv[]) {  
    char str[] = "abcdefg";

    printf("strSource=%s\n", str);
    change(str);
    printf("strResult=%s\n", str);
    return 0;
}  
