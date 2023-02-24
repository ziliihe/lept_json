#include <stdio.h>
#include <stdlib.h>


int main()
{
    char* a = "100ABCHIJK";
    char* endptr;

    long int num = strtol(a, &endptr, 16);
    printf("ret = %x, endstr = %s\n", num, endptr);
    return 0;
}