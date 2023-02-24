#include <stdio.h>

int main()
{
    if (_WIN32)
        printf("current is windows platform, _WIN32 = %d, _WINDOWS = %d\n", _WIN32, _WIN64);
    else
        printf("what the hell operation systems??");

    return 0;
}
