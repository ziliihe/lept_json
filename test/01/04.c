#include <stdio.h>
#include <stddef.h>


typedef struct example {
    int a;
    union {
        double b;
        struct {
            char* c;
            size_t d;
        } e;
    } f;
    char g;
} se;


int main()
{
    size_t a = offsetof(struct example, a);
    size_t f = offsetof(struct example, f);
    size_t g = offsetof(struct example, g);

    se s1;
    printf("alignment int a,       offset = %zu, size: %zu\n", a, sizeof(s1.a));
    printf("alignment union f,     offset = %zu, size: %zu\n", f, sizeof(s1.f));
    printf("alignment union f.b,   offset = %zu, size: %zu\n", f, sizeof(s1.f.b));
    printf("alignment union f.e,   offset = %zu, size: %zu\n", f, sizeof(s1.f.e));
    printf("alignment union f.e.c, offset = %zu, size: %zu\n", f, sizeof(s1.f.e.c));
    printf("alignment union f.e.d, offset = %zu, size: %zu\n", f, sizeof(s1.f.e.d));
    printf("alignment union g,     offset = %zu, size: %zu\n", f, sizeof(s1.g));
    printf("all                                size: %zu\n", sizeof(s1));
    printf("double size %zu\n", sizeof(double));
    return 0;
}