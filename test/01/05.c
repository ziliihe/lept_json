#include <stddef.h>
#include <stdio.h>


typedef struct Example {
    char a;        // 1 byte
    int b;         // 4 bytes
    double c;      // 8 bytes
    char d[3];     // 3 bytes
} E;


int main()
{
    printf("E = %zu\n", sizeof(E));
    printf("a %zu\n", offsetof(E, a));
    printf("b %zu\n", offsetof(E, b));
    printf("c %zu\n", offsetof(E, c));
    printf("d %zu\n", offsetof(E, d));
    return 0;
}