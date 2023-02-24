#include <stdio.h>

int main()
{
    printf("%x\n", 0xE0 | ((0x20AC >> 12) & 0xFF));
    return 0;
}

