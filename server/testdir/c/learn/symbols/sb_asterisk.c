#include <stdio.h>

struct task {
    int val;
};

int main(int argc, char *argv[])
{
    struct task atask;
    atask.val = 1;
    struct task *p = &atask;

    printf ("%d\n", *p.val);    // error .运算符优先级高于*号。
    printf ("%d\n", (*p).val);  // output: 1. OK.
    return 0;
}

