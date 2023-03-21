#include <stdio.h>
#include <string.h>

int a(int a, int b) {
    return a * a + b * b;
}

int main() {
    int num = 12345;
    char buffer[10];

    sprintf_s(buffer, sizeof(buffer), "%d", num);
    printf("The number is: %s\n", buffer);
    return 0;
}