#include "parser.h"

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
    return peek(parser)->type == type;
}

JsonObject *parseObject(Parser *parser);
JsonArray *parseArray(Parser *parser);
JsonValue *parseValue(Parser *parser);

JsonEntry *parsePair(Parser *parser)
{

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
        break;
    case JSON_NUMBER:
        printLevelIndentation(level);
        break;
    case JSON_BOOL:
        printLevelIndentation(level);
        break;
    case JSON_NULL:
        printLevelIndentation(level);
        break;

    default:
        break;
    }
}

JsonValue *parser(Lexer *lexer)
{
    Parser *p = (Parser *)malloc(sizeof(Parser));

    p->count = lexer->count;
    p->current = 0;
    p->tokens = lexer->tokens;

    JsonValue *jv = parseValue(p);

    return jv;
}
