#include <stdio.h>

static const char* lept_parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9')      *u |= ch - '0';
        else if (ch >= 'a' && ch <= 'f') *u |= ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') *u |= ch - 'A' + 10;
        else return NULL; /* I can't get it anymore until see it !!!!!! */

    }
    return p;
}

int main()
{
    char* a = "10ff";
    unsigned ret = 0;
    const char* b;
    b = lept_parse_hex4(a, &ret);
    printf("%x\n", ret);
    printf("%x\n", 0x3ff);

    printf("%x\n", 'a' - 'a' + 10);
    printf("%x\n", 'f' - 'a' + 10);
    return 0;
}