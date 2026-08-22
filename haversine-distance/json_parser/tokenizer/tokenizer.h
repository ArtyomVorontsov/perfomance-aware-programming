#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define IS_DEV false

typedef enum
{
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,

    TOKEN_STRING,
    TOKEN_NUMBER,

    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    TOKEN_EOF
} TOKEN_TYPE;

extern const char *TOKEN_NAMES[];

typedef struct
{
    char *value;
    TOKEN_TYPE type;
    size_t length;
} Token;

typedef struct
{
    FILE *file;
    Token **tokens;
    size_t count;
    size_t capacity;
} Lexer;

Lexer *lexer(FILE *fileDescriptor);

#endif