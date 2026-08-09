#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../tokenizer/tokenizer.h"

typedef enum
{
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_NULL
} JsonType;

typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;

typedef struct
{
    JsonType type;

    union
    {
        struct JsonObject *object;
        struct JsonArray *array;
        char *string;
        double number;
        bool boolean;
    } as;
} JsonValue;

typedef struct
{
    char *key;
    JsonValue *value;
} JsonEntry;

struct JsonObject
{
    JsonEntry **entries;
    size_t count;
    size_t capacity;
};

struct JsonArray
{
    JsonValue **items;
    size_t count;
    size_t capacity;
};

struct Parser
{
    Token **tokens;
    size_t count;
    size_t current;
};

#endif
