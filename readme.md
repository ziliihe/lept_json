## [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial)

```cmake
cmake -Bbuild -H.
cmake --build build
```

### 01
`json`中有 6 种数据类型:
- null
- 布尔值: true | false
- 数字（浮点数表示）
- 字符串
- 数组
- json对象
```c
typedef enum {
    LEPT_NULL,
    LEPT_FALSE, LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
} lept_type;
```
主要数据结构--节点
```c
typedef struct {
    lept_type type;
} lept_value;
```

关于类似`null x`存在冗余字符的判断
```c
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    int ret;
    /* 定义在这里，而不是每种类型的解析函数单独判断，这里将异常处理提升了一个层级 */
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0')
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}
```

对于字符的匹配，这里是按数组索引访问，因为是`null-terminate-string`最后结尾是`\0`不会满足下述判断，短路不会比较后面的，也就不会造成越界访问。
```c
if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
```