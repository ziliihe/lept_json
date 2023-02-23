#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("char -> %zu\n", sizeof(char));
    char* a = "xe1";
    double b = strtod(a, NULL);
    printf("%lf\n", b);

    return 0;
}