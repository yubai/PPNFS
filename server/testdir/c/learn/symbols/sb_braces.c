#include <stdio.h>

int main(int argc, char *argv[])
{
//    char a[10]  {= "abcde"};  // error
    char a[10]  = {"abcde"};  // OK
    int i;
    for (i = 0; a[i]; i++) {
        printf ("%c", a[i]);
    }
    printf ("\n");
    return 0;
}

