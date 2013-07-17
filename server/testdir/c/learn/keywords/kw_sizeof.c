#include <stdio.h>

int main(int argc, char *argv[])
{
    int val = 1;

    printf ("%d\n", sizeof val);  // output: 4

    int *p = NULL; 
    printf ("%d\n", sizeof (p));  // output: 4
    printf ("%d\n", sizeof(*p));  // output: 4

    int a[100];
    printf ("%d\n", sizeof (a));  // output: 400 -- the whole size of an array
    printf ("%d\n", sizeof (a[100]));  // output: 4
    printf ("%d\n", sizeof (&a));  // output: 4
    printf ("%d\n", sizeof (&a[0]));  // output: 4

    int b[100];
    printf ("%d\n", fun(b));  // output: 4 -- the optimization of the compiler
    return 0;
}

int fun(int b[100]) 
{ 
    return sizeof(b);
} 
