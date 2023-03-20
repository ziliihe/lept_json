#include <stdio.h>

int main() {
    float f = 3.14;
    char str[] = "hello";
    char buf[100];

    sprintf(buf, "f = %f, str = %s", f, str);

    printf("%s", buf);
    return 0;
}