#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("hezili == hezili %d\n", "hezili" == "hezili");
    printf("hezili == zhanghui %d\n", "hezili" == "zhanghui");

    char* a = "+1.0";
    char* b;

    double ret;
    ret = strtod(a, &b);
    printf("%lf --> %s\n", ret, b);
    return 0;
}