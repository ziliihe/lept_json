#ifndef LEPT_JSON_H_
#define LEPT_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *
 * json objects
 * six - types
 *
 */
typedef enum {
    LEPT_NULL,
    LEPT_FALSE, LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
} lept_type;

/*
 *
 * parse status
 *
 */
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

/*
 *
 * json object is a tree
 * json node
 *
 */
typedef struct {
    double n;
    lept_type type;
} lept_value;

/*
 *
 * open api
 * access to parse json string
 *
 */
int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);
double lept_get_number(const lept_value* v);
#ifdef __cplusplus
}
#endif

#endif