#include <stdio.h>
#include <string.h>

int main() {
    int num = 12345;
    char buffer[10];

    sprintf_s(buffer, sizeof(buffer), "%d", num);
    printf("The number is: %s\n", buffer);
    return 0;
}