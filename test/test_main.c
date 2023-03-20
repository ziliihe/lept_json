#include <leptjson.h>
#include <stdio.h>
#include <stdlib.h>


int main() {
    lept_value o;
    size_t index;
    lept_init(&o);

    lept_parse(&o, "{\"name\": \"zhang xiao hui\", \"gender\": \"girl\"}");
    index = lept_find_object_index(&o, "name", 4);
    if ( index != LEPT_KEY_NOT_EXIST ) {
        lept_value* v = lept_get_object_value(&o, index);
        printf("%s\n", lept_get_string(v));
    }

    lept_value* v = lept_find_object_value(&o, "gender", 6);
    if ( v != NULL ) {
        printf("she is a %s\n", lept_get_string(v));
    }

    lept_free(&o);

    const char* json = "{\"a\":[1,2],\"b\":3}";
    lept_value b;
    lept_init(&b);
    lept_parse(&b, json);
    lept_copy(
        lept_find_object_value(&b, "b", 1),
        lept_find_object_value(&b, "a", 1) );
    size_t length;
    char *out;
    lept_stringify(&b, &out, &length);
    printf("[copy] out = %s\n", out);
    free(out);

    lept_parse(&b, json);
    lept_move(
        lept_find_object_value(&b, "b", 1),
        lept_find_object_value(&b, "a", 1) );
    
    lept_stringify(&b, &out, &length);
    printf("[move] out = %s\n", out);
    free(out);

    lept_parse(&b, json);
    lept_swap(
        lept_find_object_value(&b, "b", 1),
        lept_find_object_value(&b, "a", 1) );
    
    lept_stringify(&b, &out, &length);
    printf("[swap] out = %s\n", out);
    free(out);

    lept_free(&b);
    return 0;
}