#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define IS_DEV true

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

const char *TOKEN_NAMES[] = {
    "TOKEN_LBRACE",
    "TOKEN_RBRACE",
    "TOKEN_LBRACKET",
    "TOKEN_RBRACKET",
    "TOKEN_COLON",
    "TOKEN_COMMA",
    "TOKEN_STRING",
    "TOKEN_NUMBER",
    "TOKEN_TRUE",
    "TOKEN_FALSE",
    "TOKEN_NULL",
    "TOKEN_EOF"};

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

void printDev(const char *msg)
{
    if (IS_DEV)
        printf((const char *)msg);
}

bool matchChars(FILE *fd, int *stringLength)
{
    printDev((char *)"charsFA\n");

    int c;
    int i = 0;
    do
    {
        c = getc(fd);

        if (c == '"')
        {
            fseek(fd, -1, SEEK_CUR);
            printDev((char *)"charsFA true\n");

            *stringLength = i;
            return true;
        }

        i++;

    } while (c >= 0 && c <= 127);

    fseek(fd, -i, SEEK_CUR);

    printDev((char *)"charsFA false\n");

    return false;
}

bool matchNumber(FILE *fd, int *len)
{
    printDev((char *)"numberFA\n");
    long initialFilePointer = ftell(fd);
    int c;
    int i = 0;
    bool isNumber = false;

    c = getc(fd);

    while (isdigit(c))
    {
        c = getc(fd);
        isNumber = true;
        i++;
    }

    if (!isNumber)
    {
        fseek(fd, initialFilePointer, SEEK_SET);
        printDev((char *)"numberFA false NaN\n");
        return false;
    }

    fseek(fd, -1, SEEK_CUR);
    printDev((char *)"numberFA true\n");

    *len = i;

    return true;
}

int matchString(FILE *fileDescriptor, int *stringLength)
{
    printf("matchString\n");
    if (getc(fileDescriptor) == '"' &&
        matchChars(fileDescriptor, stringLength) &&
        getc(fileDescriptor) == '"')
    {
        *stringLength = *stringLength + 2;
        return true;
    }

    return false;
}

int matchKeyword(FILE *fileDescriptor, const char *keyword, const char *name)
{
    printf("%s\n", name);
    int i = 0;
    while (*(keyword + i) != '\0')
    {
        if (getc(fileDescriptor) != *(keyword + i))
        {
            printf("%s false\n", name);
            fseek(fileDescriptor, -(i + 1), SEEK_CUR);
            return false;
        }
        i++;
    }

    printf("%s true\n", name);
    return true;
}

void addToken(char *value, TOKEN_TYPE tokenType, Lexer *lexer)
{
    Token *token = (Token *)malloc(sizeof(Token));

    token->type = tokenType;
    token->value = value;
    token->length = strlen(value);

    lexer->tokens[lexer->count] = token;
    lexer->count++;
}

void printTokens(Lexer *lexer)
{
    for (int i = 0; i < lexer->count; i++)
    {
        printf("value: %s\n", lexer->tokens[i]->value);
        printf("type: %s\n", TOKEN_NAMES[lexer->tokens[i]->type]);

        printf("\n");
    }
}

void increaseTokensBuffer(Lexer *lexer)
{
    if (lexer->count == lexer->capacity)
    {
        lexer->capacity += 100;
        lexer->tokens =
            (Token **)realloc(lexer->tokens, lexer->capacity * sizeof(Token *));
    }
}

Lexer *lexer(FILE *fileDescriptor)
{
    // create lexer
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->count = 0;
    lexer->capacity = 100;
    lexer->file = fileDescriptor;
    lexer->tokens = (Token **)malloc(sizeof(Token *) * lexer->capacity);

    int c;
    while ((c = getc(fileDescriptor)) != EOF)
    {
        increaseTokensBuffer(lexer);

        int stringLength = 0;
        printf("Character: %c (%d), location: %ld\n", c, c, ftell(fileDescriptor));
        if (c == '{')
        {
            addToken(strdup("{"), TOKEN_LBRACE, lexer);
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c == '}')
        {
            addToken(strdup("}"), TOKEN_RBRACE, lexer);
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c == '[')
        {
            addToken(strdup("["), TOKEN_LBRACKET, lexer);
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c == ']')
        {
            addToken(strdup("]"), TOKEN_RBRACKET, lexer);
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c == ',')
        {
            addToken(strdup(","), TOKEN_COMMA, lexer);
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c == ':')
        {
            addToken(strdup(":"), TOKEN_COLON, lexer);
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c == 't')
        {
            ungetc(c, fileDescriptor);

            if (matchKeyword(fileDescriptor, "true", "matchTrue"))
            {
                ungetc(c, fileDescriptor);
                addToken(strdup("true"), TOKEN_TRUE, lexer);
                printf("position %ld\n", ftell(fileDescriptor));
            }
        }
        else if (c == 'f')
        {
            ungetc(c, fileDescriptor);

            if (matchKeyword(fileDescriptor, "false", "matchFalse"))
            {
                ungetc(c, fileDescriptor);
                addToken(strdup("false"), TOKEN_FALSE, lexer);
                printf("position %ld\n", ftell(fileDescriptor));
            }
        }
        else if (c == 'n')
        {
            ungetc(c, fileDescriptor);

            if (matchKeyword(fileDescriptor, "null", "matchNull"))
            {
                ungetc(c, fileDescriptor);
                addToken(strdup("null"), TOKEN_NULL, lexer);
                printf("position %ld\n", ftell(fileDescriptor));
            }
        }
        else if (isspace(c))
        {
            // remove space
            printf("position %ld\n", ftell(fileDescriptor));
        }
        else if (c >= '0' && c <= '9')
        {
            ungetc(c, fileDescriptor);
            if (matchNumber(fileDescriptor, &stringLength))
            {
                fseek(fileDescriptor, -stringLength, SEEK_CUR);

                char *str = (char *)malloc(sizeof(char) * (stringLength + 1));

                for (int i = 0; i < stringLength; i++)
                {
                    *(str + i) = getc(fileDescriptor);
                }
                str[stringLength] = '\0';

                addToken(str, TOKEN_NUMBER, lexer);

                printf("position %ld\n", ftell(fileDescriptor));
            }
        }
        else if (c == '"')
        {
            ungetc(c, fileDescriptor);

            if (matchString(fileDescriptor, &stringLength))
            {
                fseek(fileDescriptor, -stringLength, SEEK_CUR);

                char *str = (char *)malloc(sizeof(char) * (stringLength + 1));

                for (int i = 0; i < stringLength; i++)
                {
                    *(str + i) = getc(fileDescriptor);
                }

                str[stringLength] = '\0';

                addToken(str, TOKEN_STRING, lexer);

                printf("position %ld\n", ftell(fileDescriptor));
            }
        }
        else
        {
            c = getc(fileDescriptor);

            printf("Invalid character: %c (%d), location: %ld\n", c, c, ftell(fileDescriptor));
            break;
        }
    }

    printTokens(lexer);

    return lexer;
}

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

Token *peek(Parser *parser)
{
    return parser->tokens[parser->current];
}

Token *advance(Parser *parser)
{
    Token *token = parser->tokens[parser->current];
    parser->current++;
    return token;
}

bool match(Parser *parser, TOKEN_TYPE type)
{
    printf("match %s\n", TOKEN_NAMES[type]);
    return peek(parser)->type == type;
}

JsonObject *parseObject(Parser *parser);
JsonArray *parseArray(Parser *parser);
JsonValue *parseValue(Parser *parser);

JsonEntry *parsePair(Parser *parser)
{
    printf("parsePair\n");

    Token *key;

    JsonEntry *je = (JsonEntry *)malloc(sizeof(JsonEntry));

    if (match(parser, TOKEN_STRING) &&
        (key = advance(parser)))
    {
        je->key = key->value;
    }
    else
    {
        printf("Exit 1\n");
        exit(1);
    }

    if (match(parser, TOKEN_COLON) &&
        advance(parser))
    {
    }
    else
    {
        printf("Exit 1\n");
        exit(1);
    }

    je->value = parseValue(parser);

    return je;
}

JsonObject *parseObject(Parser *parser)
{

    JsonObject *jo = (JsonObject *)malloc(sizeof(JsonObject));

    if (!jo)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    jo->count = 0;
    jo->capacity = 32;
    JsonEntry **ne = (JsonEntry **)malloc(sizeof(JsonEntry *) * jo->capacity);

    if (!ne)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    jo->entries = ne;

    printf("parseObject\n");
    if (match(parser, TOKEN_LBRACE))
    {
        advance(parser);
    }
    else
    {
        printf("Exit 1\n");
        exit(1);
    }

    while ((jo->entries[jo->count++] = parsePair(parser)) &&
           (match(parser, TOKEN_COMMA) && advance(parser)))
    {
        if (jo->capacity == jo->count)
        {
            jo->capacity *= 2;

            JsonEntry **ne = (JsonEntry **)realloc(jo->entries, sizeof(JsonEntry *) * jo->capacity);

            if (!ne)
            {
                perror("realloc");
                exit(EXIT_FAILURE);
            }

            jo->entries = ne;
        }
    }

    if (match(parser, TOKEN_RBRACE))
    {
        advance(parser);
    }
    else
    {
        printf("Exit 1\n");
        exit(1);
    }

    return jo;
}

JsonArray *parseArray(Parser *parser)
{
    printf("parseArray\n");

    JsonArray *ja = (JsonArray *)malloc(sizeof(JsonArray));

    if (!ja)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    ja->capacity = 32;
    ja->count = 0;

    JsonValue **ni = (JsonValue **)malloc(sizeof(JsonValue *) * ja->capacity);

    if (!ni)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    ja->items = ni;

    if (match(parser, TOKEN_LBRACKET))
    {
        advance(parser);
    }
    else
    {
        printf("Exit 1\n");
        exit(1);
    }

    // Handle empty array
    if (match(parser, TOKEN_RBRACKET))
    {
        advance(parser);
    }
    else
    {
        while (((ja->items[ja->count++] = parseValue(parser))) &&
               (match(parser, TOKEN_COMMA) && advance(parser)))
        {
            if (ja->capacity == ja->count)
            {
                ja->capacity *= 2;
                JsonValue **ni = (JsonValue **)realloc(ja->items, sizeof(JsonValue *) * ja->capacity);

                if (!ni)
                {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }

                ja->items = ni;
            }
        }

        if (match(parser, TOKEN_RBRACKET))
        {
            advance(parser);
        }
        else
        {
            printf("Exit 1\n");
            exit(1);
        }
    }

    return ja;
}

JsonValue *parseValue(Parser *parser)
{

    JsonValue *jv = (JsonValue *)malloc(sizeof(JsonValue));

    if (!jv)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    printf("parseValue\n");
    if (match(parser, TOKEN_LBRACE))
    {
        JsonObject *jo = parseObject(parser);

        jv->as.object = jo;
        jv->type = JSON_OBJECT;
        return jv;
    }

    if (match(parser, TOKEN_LBRACKET))
    {
        JsonArray *ja = parseArray(parser);

        jv->as.array = ja;
        jv->type = JSON_ARRAY;
        return jv;
    }

    if (match(parser, TOKEN_NUMBER))
    {
        Token *n = advance(parser);
        jv->as.number = strtod(n->value, NULL);
        jv->type = JSON_NUMBER;

        return jv;
    }

    if (match(parser, TOKEN_STRING))
    {
        Token *n = advance(parser);
        jv->as.string = n->value;
        jv->type = JSON_STRING;

        return jv;
    }

    if (match(parser, TOKEN_TRUE))
    {
        Token *n = advance(parser);
        jv->as.boolean = true;
        jv->type = JSON_BOOL;

        return jv;
    }

    if (match(parser, TOKEN_FALSE))
    {
        Token *n = advance(parser);
        jv->as.boolean = false;
        jv->type = JSON_BOOL;

        return jv;
    }

    if (match(parser, TOKEN_NULL))
    {
        Token *n = advance(parser);
        jv->type = JSON_NULL;

        return jv;
    }

    printf("Exit 1\n");
    exit(1);
}

void printValue(JsonValue *jsonValue, size_t level);

void printObject(JsonObject *object, size_t level)
{
    for (size_t i = 0; i < object->count; i++)
    {
        printValue(object->entries[i]->value, level + 1);
    }
}

void printArray(JsonArray *array, size_t level)
{
    for (size_t i = 0; i < array->count; i++)
    {
        printValue(array->items[i], level + 1);
    }
}

void printLevelIndentation(int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("\t");
    }
}

void printValue(JsonValue *jsonValue, size_t level)
{
    switch (jsonValue->type)
    {

    case JSON_OBJECT:
        printObject(jsonValue->as.object, level);
        break;
    case JSON_ARRAY:
        printArray(jsonValue->as.array, level);
        break;
    case JSON_STRING:
        printLevelIndentation(level);
        printf("string: %s\n", jsonValue->as.string);
        break;
    case JSON_NUMBER:
        printLevelIndentation(level);
        printf("number: %f\n", jsonValue->as.number);
        break;
    case JSON_BOOL:
        printLevelIndentation(level);
        printf("boolean: %s\n", jsonValue->as.boolean ? "true" : "false");
        break;
    case JSON_NULL:
        printLevelIndentation(level);
        printf("null\n");
        break;

    default:
        break;
    }
}

void parser(Lexer *lexer)
{
    Parser *p = (Parser *)malloc(sizeof(Parser));

    p->count = lexer->count;
    p->current = 0;
    p->tokens = lexer->tokens;

    JsonValue *jv = parseValue(p);

    printf("type: %d\n", jv->type);

    printValue(jv, 0);
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printDev((char *)"No json file provided.");
        return 1;
    }

    char *fileName = argv[1];

    FILE *fileDescriptor = fopen(fileName, "r");

    Lexer *lexerResult = lexer(fileDescriptor);
    parser(lexerResult);

    return 0;
}
