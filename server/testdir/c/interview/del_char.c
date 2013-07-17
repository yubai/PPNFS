/**
 * 删除串中指定的字符（做此题时，千万不要开辟新空间，否则面试官可能认为你不适合做嵌入式开发）
 */

#include <stdio.h>
#include <string.h>

void delChar(char *str, char c) {  
     int i, j=0;  
     for(i=0; str[i]; i++)  
         if(str[i]!=c) str[j++]=str[i];  
     str[j] = '\0';  
}  
   
int main(int argc, char* argv[]) {  
     char str[] = "abcdefgh";    // 注意，此处不能写成char *str = "abcdefgh";  
     printf("%s\n", str);  
     delChar(str, 'c');  
     printf("%s\n", str);  
}
