#include <gtest/gtest.h>
#include <leptjson.h>
#include <iostream>

TEST(leptjson, tnull) {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
}

TEST(leptjson, expect_value) {
    lept_value v;
    v.type = LEPT_FALSE;

    EXPECT_EQ(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, "\n\t\r"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, "    "));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
}

TEST(leptjson, invalid) {
    lept_value v;
    v.type = LEPT_FALSE;

    EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nuil"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "nullx"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "?"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "tr"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "fals"));
    EXPECT_EQ(LEPT_NULL, lept_get_type(&v));
}

TEST(leptjson, boolean) {
    lept_value v;
    lept_init(&v);
    v.type = LEPT_NULL;

    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ(LEPT_TRUE, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ(LEPT_FALSE, lept_get_type(&v));

    lept_set_boolean(&v, 23);
    EXPECT_EQ(lept_get_boolean(&v), 1);
    lept_set_boolean(&v, 0);
    EXPECT_EQ(lept_get_boolean(&v), 0);
}

#define TEST_NUMBER(expect, json) \
    do {\
        lept_value v;\
        lept_init(&v); \
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_DOUBLE_EQ(expect, lept_get_number(&v));\
        lept_free(&v);\
    } while(0)

#define TEST_ERROR(error, json)\
    do {\
        lept_value v;\
        lept_init(&v); \
        v.type = LEPT_FALSE;\
        EXPECT_EQ(error, lept_parse(&v, json));\
        EXPECT_EQ(LEPT_NULL, lept_get_type(&v));\
        lept_free(&v);\
    } while(0)


TEST(leptjson, number) {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(1.7976931348623157E308, "1.7976931348623157E308");
    TEST_NUMBER(2.2250738585072009E-308, "2.2250738585072009E-308");
    TEST_NUMBER(2.2250738585072014E-308, "2.2250738585072014E-308");
    TEST_NUMBER(4.9406564584124654E-324, "4.9406564584124654E-324");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
    lept_value v;
    lept_init(&v);
    lept_set_number(&v, 3.33333333333333);
    EXPECT_DOUBLE_EQ(lept_get_number(&v), 3.33333333333333);
}

TEST(leptjson, invalid_num) {
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
}


#define TEST_STRING(expect, json) \
    do {\
        lept_value v;\
        lept_init(&v);\
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ(LEPT_STRING, lept_get_type(&v));\
        EXPECT_STREQ(expect, lept_get_string(&v));\
        lept_free(&v);\
    } while(0)


TEST(leptjson, string) {
    EXPECT_STREQ("hello", "hello");
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");


    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */


}

TEST(leptjson, string_error) {
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");

    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");

    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}


TEST(leptjson, array) {
    size_t a = 30;
    size_t b = 30;
    size_t c = 31;

    EXPECT_EQ(a, b);
    EXPECT_NE(b, c);

    lept_value v;
    lept_init(&v);

    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "[ ]"));
    EXPECT_EQ(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ(0, lept_get_array_size(&v));
    lept_free(&v);

    /* [ null , false , true , 123 , "abc" ] */
    lept_init(&v);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ(5, lept_get_array_size(&v));
    EXPECT_EQ(LEPT_NULL, lept_get_type(lept_get_array_element(&v, 0)));
    EXPECT_EQ(LEPT_FALSE, lept_get_type(lept_get_array_element(&v, 1)));
    EXPECT_EQ(LEPT_TRUE, lept_get_type(lept_get_array_element(&v, 2)));
    EXPECT_EQ(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 3)));
    EXPECT_EQ(123, lept_get_array_element(&v, 3)->u.n);
    EXPECT_EQ(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 4)));
    EXPECT_STREQ("abc", lept_get_array_element(&v, 4)->u.s.s);


    /* [ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ] */
    lept_init(&v);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ(4, lept_get_array_size(&v));
    for (int i = 0; i < 4; i++) {
        lept_value* v2 = lept_get_array_element(&v, i);
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(v2));
        EXPECT_EQ(i, lept_get_array_size(lept_get_array_element(&v, i)));
        for (int j = 0; j < i; j++) {
            // std::cout << "[i] = " << i << ", [j] = " << j << std::endl;
            
            EXPECT_EQ(LEPT_NUMBER, lept_get_type(lept_get_array_element(v2, j)));
            EXPECT_EQ(j, lept_get_array_element(v2, j)->u.n);
        }
    }
}

TEST(leptjson, object) {
     lept_value v;
    size_t i;

    lept_init(&v);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, " { } "));
    EXPECT_EQ(LEPT_OBJECT, lept_get_type(&v));
    EXPECT_EQ(0, lept_get_object_size(&v));
    lept_free(&v);

    lept_init(&v);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v,
        " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [ 1, 2, 3 ],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "
    ));
    EXPECT_EQ(LEPT_OBJECT, lept_get_type(&v));
    EXPECT_EQ(7, lept_get_object_size(&v));
    EXPECT_STREQ("n", lept_get_object_key(&v, 0));
    EXPECT_EQ(LEPT_NULL, lept_get_type(lept_get_object_value(&v, 0)));
    EXPECT_STREQ("f", lept_get_object_key(&v, 1));
    EXPECT_EQ(LEPT_FALSE,  lept_get_type(lept_get_object_value(&v, 1)));
    EXPECT_STREQ("t", lept_get_object_key(&v, 2));
    EXPECT_EQ(LEPT_TRUE,   lept_get_type(lept_get_object_value(&v, 2)));
    EXPECT_STREQ("i", lept_get_object_key(&v, 3));
    EXPECT_EQ(LEPT_NUMBER, lept_get_type(lept_get_object_value(&v, 3)));
    EXPECT_DOUBLE_EQ(123.0, lept_get_number(lept_get_object_value(&v, 3)));
    EXPECT_STREQ("s", lept_get_object_key(&v, 4));
    EXPECT_EQ(LEPT_STRING, lept_get_type(lept_get_object_value(&v, 4)));
    EXPECT_STREQ("abc", lept_get_string(lept_get_object_value(&v, 4)));
    EXPECT_STREQ("a", lept_get_object_key(&v, 5));
    EXPECT_EQ(LEPT_ARRAY, lept_get_type(lept_get_object_value(&v, 5)));
    EXPECT_EQ(3, lept_get_array_size(lept_get_object_value(&v, 5)));
    for (i = 0; i < 3; i++) {
        lept_value* e = lept_get_array_element(lept_get_object_value(&v, 5), i);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e));
        EXPECT_DOUBLE_EQ(i + 1.0, lept_get_number(e));
    }
    EXPECT_STREQ("o", lept_get_object_key(&v, 6));
    lept_value* o = lept_get_object_value(&v, 6);
    EXPECT_EQ(LEPT_OBJECT, lept_get_type(o));
    for (i = 0; i < 3; i++) {
        lept_value* ov = lept_get_object_value(o, i);
        EXPECT_TRUE('1' + i == lept_get_object_key(o, i)[0]);
        EXPECT_EQ(1, lept_get_object_key_length(o, i));
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(ov));
        EXPECT_DOUBLE_EQ(i + 1.0, lept_get_number(ov));
    }
    lept_free(&v);
}

#define TEST_ROUNDTRIP(json)\
    do {\
        lept_value v;\
        char* json2;\
        size_t length;\
        lept_init(&v);\
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ(LEPT_STRINGIFY_OK, lept_stringify(&v, &json2, &length));\
        EXPECT_STREQ(json, json2);\
        lept_free(&v);\
        free(json2);\
    } while(0)

TEST(leptjson, stringify) {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
}