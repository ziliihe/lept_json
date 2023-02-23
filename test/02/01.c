#include <stdio.h>

#define TOUPPER(ch)  ((ch) - 'A') >= 32 ? (ch) - 32 : (ch)
int main()
{
    printf("a --> %d\n", 'a');
    printf("A --> %d\n", 'A');
    printf("0 --> %d\n", '0');
    printf("a - A --> %d\n", 'a' - 'A');
    printf("a - 32 --> %c\n", 'a' - 32);
    printf("toupper(a) = %c\n", TOUPPER('a'));
    printf("toupper(A) = %c\n", TOUPPER('A'));
    printf("toupper(C) = %c\n", TOUPPER('C'));
    printf("toupper(x) = %c\n", TOUPPER('x'));
    printf("toupper(z) = %c\n", TOUPPER('z'));
    return 0;
}