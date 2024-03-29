#include "leptjson.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>  /* strtod strtol */
#include <errno.h>
#include <math.h>
#include <stdio.h>

/*
 *
 * windows detect memory leak
 *
 */
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#else
#include <unistd.h>
#endif


#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif
#define PUTC(c, ch) do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

#ifndef LEPT_PARSE_STRINGIFY_INIT_SIZE
#define LEPT_PARSE_STRINGIFY_INIT_SIZE 256
#endif
/* avoid multi para transfermation (char array) */
typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
} lept_context;

/* ws = *(%x20 / %x09 d:\Microsoft VS Code\resources\app\out\vs\code\electron-sandbox\workbench\workbench.html/ %x0A / %x0D) */
static void lept_parse_whitespace(lept_context* c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type){
    EXPECT(c, literal[0]);

    size_t i;
    for (i = 0; literal[i+1]; i++) {
        if (c->json[i] != literal[i+1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

/* number */
static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p);p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p); p++);
    }

    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '-' || *p == '+') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p; /* not reset c->json, will cause return LEPT_PARSE_ROOT_NOT_SINGULAR*/
    return LEPT_PARSE_OK;
}

/* string */
void lept_free(lept_value* v) {
    assert(v != NULL);

    size_t i;
    switch ( v->type ) {
        case LEPT_ARRAY:
            for (i = 0; i < v->u.a.size; i++) {
                lept_free( &v->u.a.e[i] );
            }
            free( v->u.a.e );
            break;
            
        case LEPT_OBJECT:
            for (i = 0;i < v->u.o.size; i++) {
                free(v->u.o.m[i].k);
                lept_free(&v->u.o.m[i].v);
            }
            free(v->u.o.m);
            break;
        case LEPT_STRING:
            free( v->u.s.s );
            break;
        
        default:
            break;
    }
    v->type = LEPT_NULL;
}


/* after push, return data start pointer */
static void* lept_context_push(lept_context* c, size_t size) {
    void* ret;
    assert(size > 0);

    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
         /* dynamic factor is 1.5 */
        while (c->top + size >= c->size)
            c->size += c->size >> 1; 
        
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* lept_context_pop(lept_context* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static const char* lept_parse_hex4(const char* p, unsigned* u) {
    *u = 0;
    int i;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9')      *u |= ch - '0';
        else if (ch >= 'a' && ch <= 'f') *u |= (ch - 'a') + 10;
        else if (ch >= 'A' && ch <= 'F') *u |= (ch - 'A') + 10;
        else return NULL; 
    }
    return p;
    /*
    char* endptr;
    *u =  (unsigned) strtol(p, &endptr, 16);
    return (p + 4) == endptr ? p : NULL;
    */
}

static void lept_encode_utf8(lept_context* c, unsigned u) {
    if (u <= 0x7F) {
        PUTC(c, u & 0x7F);
    } else if ( u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF)); /* 0xC0 = 1100 0000 */
        PUTC(c, 0x80 | (u        & 0x3F));
    } else if ( u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
        PUTC(c, 0x80 | (u         & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 21) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
        PUTC(c, 0x80 | (u         & 0x3F));
    }
}


static int lept_parse_string_raw(lept_context* c, char** str, size_t* len) {
    size_t head = c->top;
    const char* p;
    unsigned u;
    unsigned u2;
    /* start string */
    EXPECT(c, '\"');
    p = c->json;
    for(;;) {
        char ch = *p++;
        switch(ch) {
            /* end string */
            case '\"':
                *len = c->top - head;
                /* here set the type and the remain... */
                *str = (char*)lept_context_pop(c, *len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\0':
                STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);

            case '\\':
                switch(*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/');  break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':
                        if (!(p = lept_parse_hex4(p, &u)))
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if ( u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = lept_parse_hex4(p, &u2)))
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 >0xDFFF)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            
                            u = 0x10000 + (u - 0xd800) * 0x400 + (u2 - 0xdc00);
                        }
                        /* printf("[ Unicode %x ]\n", u); */
                        lept_encode_utf8(c, u);
                        break;
                             
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            default:
                if ((unsigned char) ch < 0x20) {
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                }
                PUTC(c, ch);
        }

    }

    return LEPT_PARSE_OK;
}

static int lept_parse_string(lept_context* c, lept_value* v) {
    int ret;
    char* s;
    size_t len;
    if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK)
        lept_set_string(v, s, len);
    return ret;
}

static int lept_parse_value(lept_context* c, lept_value* v); /* forward declaration */
static int lept_parse_array(lept_context* c, lept_value* v) {
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    lept_parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = LEPT_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return LEPT_PARSE_OK;
    }

    for (;;) {
        lept_value e;
        lept_init(&e);

        if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK)
            return ret;
        
        memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        size++;

        lept_parse_whitespace(c);
        if (*c->json == ',')
            c->json++;
        else if (*c->json == ']') {
            c->json++;
            v->type = LEPT_ARRAY;
            v->u.a.size = size;
            size *= sizeof(lept_value);
            memcpy(v->u.a.e = (lept_value*) malloc(size), lept_context_pop(c, size), size);

            return LEPT_PARSE_OK;
        } else {
            return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }
}


static int lept_parse_object(lept_context* c, lept_value* v) {
    size_t size;
    lept_member m;
    int ret;
    EXPECT(c, '{');
    lept_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = LEPT_OBJECT;
        v->u.o.m = 0;
        v->u.o.size = 0;
        return LEPT_PARSE_OK;
    }
    m.k = NULL;
    size = 0;
    for (;;) {
        char* str;
        lept_init(&m.v);
        /* \todo parse key to m.k, m.klen */
        if (*c->json != '"') {
            ret = LEPT_PARSE_MISS_KEY;
            break;
        }

        if (( ret = lept_parse_string_raw( c, &str, &m.klen )) != LEPT_PARSE_OK ) {
            break;
        }
        memcpy(m.k = (char*) malloc(m.klen + 1), str, m.klen );
        m.k[m.klen] = '\0';
        lept_parse_whitespace(c);
        if (*c->json != ':') {
            ret = LEPT_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        lept_parse_whitespace(c);
        /* parse value */
        if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK)
            break;
        memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
        size++;
        m.k = NULL; /* ownership is transferred to member on stack */
        /* \todo parse ws [comma | right-curly-brace] ws */
        lept_parse_whitespace(c);
        if ( *c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        } else if ( *c->json == '}' ) {
            size_t s = sizeof(lept_member) * size;
            c->json++;
            v->type = LEPT_OBJECT;
            v->u.o.size = size;
            memcpy( v->u.o.m = ( lept_member* ) malloc(s), lept_context_pop(c, s), s );
            return LEPT_PARSE_OK;
        } else {
            ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
        lept_parse_whitespace(c);
    }
    /* \todo Pop and free members on the stack */
    free( m.k );
    for (int i = 0; i < size; i++) {
        lept_member* m = (lept_member*) lept_context_pop(c, sizeof(lept_member));
        free( m->k );
        lept_free( &m->v );
    }
    v->type = LEPT_NULL;
    return ret;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    lept_parse_whitespace(c);
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '"':  return lept_parse_string(c, v);
        case '[':  return lept_parse_array(c, v);
        case '{':  return lept_parse_object(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return lept_parse_number(c, v);
    }
}


int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    int ret;
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    /* printf("json = %s, c.top = %zu\n", json, c.top); */
    assert(c.top == 0);
    free(c.stack);

    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

int lept_get_boolean(const lept_value* v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
    assert(v != NULL);
    if (b)
        v->type = LEPT_TRUE;
    else
        v->type = LEPT_FALSE;
}

void  lept_set_string(lept_value* v, const char* s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);
    v->u.s.s = (char*)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

const char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}

size_t lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

size_t lept_get_array_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.a.size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

size_t lept_get_object_size(const lept_value* v) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    return v->u.o.size;
}

const char* 
lept_get_object_key( const lept_value* v, size_t index ) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    assert( index < v->u.o.size );
    return v->u.o.m[index].k;
}

size_t
lept_get_object_key_length( const lept_value* v, size_t index ) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    assert( index < v->u.o.size );
    return v->u.o.m[index].klen;
}

lept_value* 
lept_get_object_value( const lept_value* v, size_t index ) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    assert( index < v->u.o.size );
    return &v->u.o.m[index].v;
}

#define PUTS(c, s, len)     memcpy(lept_context_push(c, len), s, len)

static void lept_stringify_string(lept_context* c, const char* s, size_t len) {
    /* "Hello" -> "\"Hello\"" */
    size_t i;
    assert( s != NULL);
    PUTC(c, '"'); /* start */
    for (i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char) s[i];

        switch (ch) {
            case '\"': memcpy(lept_context_push(c, 2), "\\\"", 2); break;
            case '\\': memcpy(lept_context_push(c, 2), "\\\\", 2); break;
            case '\b': memcpy(lept_context_push(c, 2), "\\b", 2); break;
            case '\f': memcpy(lept_context_push(c, 2), "\\f", 2); break;
            case '\n': memcpy(lept_context_push(c, 2), "\\n", 2); break;
            case '\r': memcpy(lept_context_push(c, 2), "\\r", 2); break;
            case '\t': memcpy(lept_context_push(c, 2), "\\t", 2); break;
            default:
                if (ch < 0x20)  { /* control character */
                    char buffer[7];
                    sprintf_s(buffer, sizeof(buffer), "\\u%04X", ch);
                    memcpy(lept_context_push(c, 6), buffer, 6);
                } else {
                    *(char*)lept_context_push(c, sizeof(char)) = s[i];
                }
        }
    }
    *(char*)lept_context_push(c,sizeof(char)) = '"';
}

static int lept_stringify_value(lept_context* c, const lept_value* v) {
    size_t i;
    switch (v->type) {
        case LEPT_NULL:   PUTS(c, "null",  4); break;
        case LEPT_FALSE:  PUTS(c, "false", 5); break;
        case LEPT_TRUE:   PUTS(c, "true",  4); break;
        case LEPT_NUMBER: c->top -= 32 - sprintf_s(lept_context_push(c, 32), 32, "%.17g", v->u.n); break;
        case LEPT_STRING: lept_stringify_string(c, v->u.s.s, v->u.s.len); break;
        case LEPT_ARRAY:
            *(char*)lept_context_push(c, sizeof(char)) = '[';
            for (i = 0; i < v->u.a.size; ++i) {
                lept_stringify_value(c, &v->u.a.e[i]);
                if ( i + 1 == v->u.a.size )
                    continue;
                *(char*)lept_context_push(c, sizeof(char)) = ',';
            }
            *(char*)lept_context_push(c, sizeof(char)) = ']';
            break;
        case LEPT_OBJECT:
            *(char*)lept_context_push(c, sizeof(char)) = '{';
            for ( i = 0; i < v->u.o.size; ++i ) {
                lept_member* curr_mem = &v->u.o.m[i];
                PUTC(c, '"');
                PUTS(c, curr_mem->k,curr_mem->klen);
                PUTC(c, '"');
                *(char*)lept_context_push(c, sizeof(char)) = ':';
                lept_stringify_value(c, &curr_mem->v);
                if ( i + 1 == v->u.o.size)
                    continue;
                *(char*)lept_context_push(c, sizeof(char)) = ',';
            }
            *(char*)lept_context_push(c, sizeof(char)) = '}';
    }
    return LEPT_STRINGIFY_OK;
}

int 
lept_stringify( const lept_value* v, char** json, size_t* length ) {
    lept_context c;
    int ret;
    assert( v != NULL );
    assert( json != NULL );
    c.stack = (char*) malloc( c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE );
    c.top = 0;
    if (( ret = lept_stringify_value( &c, v) ) != LEPT_STRINGIFY_OK ) {
        free( c.stack );
        *json = NULL;
        return ret;
    }

    if (length) 
        *length = c.top;
    
    PUTC(&c, '\0');
    *json = c.stack;

    return LEPT_STRINGIFY_OK;
}

size_t lept_find_object_index( const lept_value* v, const char* key, size_t klen) {
    size_t i;
    assert( v != NULL && v->type == LEPT_OBJECT && key != NULL );
    for ( i = 0; i < v->u.o.size; ++i ) {
        if ( v->u.o.m[i].klen == klen && memcmp( v->u.o.m[i].k, key, klen) == 0 ) {
            return i;
        }
    }
    return LEPT_KEY_NOT_EXIST;
}

lept_value* lept_find_object_value( const lept_value* v, const char* key, size_t klen) {
    size_t index = lept_find_object_index(v, key, klen);
    return index != LEPT_KEY_NOT_EXIST ? &v->u.o.m[index].v : NULL;
}

/* 0 -- not equal; 1 -- equal */
int lept_is_equal(const lept_value* lhs, const lept_value* rhs) {
    assert( lhs != NULL && rhs != NULL );
    if (lhs->type != rhs->type ) {
        return 0;
    }

    size_t i;
    switch (lhs->type) {
        case LEPT_STRING:
            return lhs->u.s.len == rhs->u.s.len && memcmp( lhs->u.s.s, rhs->u.s.s, lhs->u.s.len) == 0;
        case LEPT_NUMBER:
            return lhs->u.n == rhs->u.n;
        case LEPT_ARRAY:
            if ( lhs->u.a.size != rhs->u.a.size )
                return 0;
            for ( i = 0; i < lhs->u.a.size; ++i ) {
                if ( !lept_is_equal( &lhs->u.a.e[i], &rhs->u.a.e[i] )) {
                    return 0;
                }
            }
            return 1;
        case LEPT_OBJECT:
            if ( lhs->u.o.size != rhs->u.o.size ) {
                return 0;
            }
            for ( i = 0; i < lhs->u.o.size; ++i ) {
                if ( lept_find_object_index( lhs, rhs->u.o.m->k, rhs->u.o.m->klen ) == LEPT_KEY_NOT_EXIST ) {
                    return 0;
                }
            }
            return 1;
        default:
            return 1;
    }
}

void lept_copy(lept_value* dst, const lept_value* src) {
    size_t i;
    assert( dst != NULL && src != NULL && src != dst );
    switch (src->type) {
        case LEPT_STRING:
            lept_set_string(dst, src->u.s.s, src->u.s.len);
            break;
        case LEPT_ARRAY:
            lept_set_array( dst, src->u.a.size );
            for (i = 0; i < src->u.a.size; ++i ) {
                lept_copy( &dst->u.a.e[i], &src->u.a.e[i] );
            }
            break;
        case LEPT_OBJECT:
            lept_set_object( dst, src->u.o.size );
            for ( i = 0; i < src->u.o.size; ++i ) {
                lept_copy( lept_set_object_value( &src->u.o.m[i].v, src->u.o.m[i].k, src->u.o.m[i].klen ), &src->u.o.m[i].v );
            }
            break;
        default:
            lept_free(dst);
            memcpy(dst, src, sizeof(lept_value));
            break;
    }
}

void lept_move(lept_value* dst, lept_value* src) {
    assert( dst != NULL && src != NULL && src != dst);
    lept_free(dst);
    memcpy(dst, src, sizeof(lept_value));
    lept_init(src);
}

void lept_swap(lept_value* lhs, lept_value* rhs) {
    assert(lhs != NULL && rhs != NULL);
    if (lhs != rhs) {
        lept_value temp;
        memcpy(&temp, lhs, sizeof(lept_value));
        memcpy(lhs, rhs, sizeof(lept_value));
        memcpy(rhs, &temp, sizeof(lept_value));
    }
}

void lept_set_array(lept_value* v, size_t capacity ) {
    assert( v!= NULL );
    lept_free(v);
    v->type = LEPT_ARRAY;
    v->u.a.size = 0;
    v->u.a.capacity = capacity;
    v->u.a.e = capacity > 0 ? (lept_value*)malloc(capacity * sizeof(lept_value)) : NULL;
}

size_t lept_get_array_capacity(const lept_value* v) {
    assert( v != NULL && v->type == LEPT_ARRAY );
    return v->u.a.capacity;
}

void lept_reserve_array(lept_value* v, size_t capacity ) {
    assert( v != NULL && v->type == LEPT_ARRAY );
    if (capacity > v->u.a.capacity ) { 
        v->u.a.capacity = capacity;
        v->u.a.e = (lept_value*) realloc(v->u.a.e, capacity * sizeof(lept_value));
    }
}

lept_value* lept_pushback_array_element(lept_value* v) {
    assert( v != NULL && v->type == LEPT_ARRAY );
    if (v->u.a.size == v->u.a.capacity) {
        lept_reserve_array( v, v->u.a.capacity == 0 ? 1 : v->u.a.capacity * 2);
    }
    lept_init(&v->u.a.e[v->u.a.size]);
    return &v->u.a.e[v->u.a.size++];
}

void lept_popback_array_element( lept_value* v ) {
    assert( v != NULL && v->type == LEPT_ARRAY && v->u.a.size > 0 );
    lept_free( &v->u.a.e[--(v->u.a.size)] );
}

lept_value*
lept_insert_array_element(lept_value* v, size_t index) {
    assert( v != NULL && v->type == LEPT_ARRAY && index >= 0 && index <= v->u.a.size );
    if ( v->u.a.size == v->u.a.capacity ) {
        lept_reserve_array( v, v->u.a.capacity == 0 ? 1 : v->u.a.capacity * 2 );
    }
    lept_init(&v->u.a.e[v->u.a.size]);
    size_t i;
    for ( i = v->u.a.size ; i > index ; --i ) {
        lept_move(&v->u.a.e[i], &v->u.a.e[i - 1]);
    }
    v->u.a.size++;
    return &v->u.a.e[i];
}

/*
 *
 * erase count elements since index
 *
 */
void lept_erase_array_element(lept_value* v, size_t index, size_t count) {
    assert( v != NULL && v->u.a.size > 0 && 
           index >= 0 && index < v->u.a.size && count > 0 );
    size_t i;
    size_t raw_size = v->u.a.size;
    size_t start = index;
    for ( i = 0; i < raw_size; ++i ) {
        if ( i < index ) {
            continue;
        } else {
            if ( i - index < count ) {
                lept_free( &v->u.a.e[i]);
                v->u.a.size--;
            } else {
                lept_swap( &v->u.a.e[start++], &v->u.a.e[i] );
            }
        }
    }
}

void lept_shrink_array( lept_value* v ) {
    assert( v != NULL && v->type == LEPT_ARRAY );
    if (v->u.a.capacity > v->u.a.size ) {
        v->u.a.capacity = v->u.a.size;
        v->u.a.e = (lept_value*) realloc(v->u.a.e, sizeof( lept_value ) * v->u.a.capacity );
    }
}

void lept_clear_array(lept_value* v) {
    assert( v != NULL && v->type == LEPT_ARRAY );
    lept_erase_array_element( v, 0, v->u.a.size );
}

void lept_set_object(lept_value* v, size_t capacity) {
    assert( v != NULL );
    lept_free( v );
    v->type = LEPT_OBJECT;
    v->u.o.size = 0;
    v->u.o.capacity = capacity;
    v->u.o.m = capacity > 0 ? (lept_member*)malloc(capacity * sizeof(lept_member)) : NULL;
}

size_t lept_get_object_capacity( const lept_value* v) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    return v->u.o.capacity;
}

void lept_reserve_object( lept_value* v, size_t capacity ) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    if ( capacity > v->u.o.capacity ) {
        v->u.o.capacity = capacity;
        v->u.o.m = (lept_member*) realloc(v->u.o.m, capacity * sizeof(lept_member) );
    }
}

void lept_shrink_object(lept_value* v) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    size_t i;

    for (i = 0; i < v->u.o.size; ++i ) {
        lept_member* m = &v->u.o.m[i];
        if ( m->v.type == LEPT_OBJECT ) {
            lept_shrink_object( &m->v );
        }

        if ( m->v.type == LEPT_ARRAY ) {
            lept_shrink_array( &m->v );
        }
    }

    if ( v->u.o.capacity > v->u.o.size ) {
        v->u.o.capacity = v->u.o.size;
        v->u.o.m = (lept_member*) realloc( v->u.o.m, sizeof( lept_member ) * v->u.o.capacity );
    }
}

void lept_clear_object(lept_value* v) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    size_t capacity = v->u.o.capacity;
    lept_free( v );
    lept_set_object( v, capacity );
}

lept_value* 
lept_set_object_value(lept_value* v, const char* key, size_t klen) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    size_t index = lept_find_object_index( v, key, klen );

    if ( index != LEPT_KEY_NOT_EXIST ) {
        return &v->u.o.m[index].v;
    }

    lept_reserve_object( v, v->u.o.size + 1 );

    lept_member* m = &v->u.o.m[v->u.o.size];
    m->k = memcpy((char*) malloc( sizeof(char) * klen ), key, klen);
    m->klen = klen;
    v->u.o.size++;

    return &m->v;
}

void lept_remove_object_value(lept_value* v, size_t index) {
    assert( v != NULL && v->type == LEPT_OBJECT );
    if ( v->u.o.size == 0 || index < 0 || index >= v->u.o.size ) {
        return;
    }

    lept_member* m = &v->u.o.m[index];
    if ( m == NULL || m->k == NULL ) {
        return;
    }
    if ( index != v->u.o.size - 1 ) {
        free( m->k );
        m->k = (char*)malloc( sizeof(char) * v->u.o.m[v->u.o.size - 1].klen );
        memcpy( m->k, v->u.o.m[v->u.o.size - 1].k, v->u.o.m[v->u.o.size-1].klen );
        free( v->u.o.m[v->u.o.size - 1].k );

        lept_swap(&m->v, &v->u.o.m[v->u.o.size - 1].v );
        lept_free( &v->u.o.m[v->u.o.size - 1].v );
        v->u.o.size--;
    } else {
        v->u.o.size--;
        m = &v->u.o.m[v->u.o.size];
        free( m->k );
        lept_free( &m->v );
    }
}