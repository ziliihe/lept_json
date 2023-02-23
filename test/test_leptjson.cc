#include <gtest/gtest.h>
#include <leptjson.h>

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
    v.type = LEPT_NULL;

    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ(LEPT_TRUE, lept_get_type(&v));
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ(LEPT_FALSE, lept_get_type(&v));
}

#define TEST_NUMBER(expect, json) \
    do {\
        lept_value v;\
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_DOUBLE_EQ(expect, lept_get_number(&v));\
    } while(0)

#define TEST_ERROR(error, json)\
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ(error, lept_parse(&v, json));\
        EXPECT_EQ(LEPT_NULL, lept_get_type(&v));\
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