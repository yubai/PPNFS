#include <stdio.h>

int main(int argc, char *argv[])
{
    char c;
    c = getchar();
    switch(c)
    {
    case 'a' :
        printf("press a\n");
        break;
    case 'b' :
        printf ("press b\n");
        break;
        
        /** We can't use continue in switch-case sentence,
         *  otherwise will occur complilation error:
         *  error: continue statement not within a loop
         */
        // continue; 
    default :
        break;
            
    }
    return 0;
}

