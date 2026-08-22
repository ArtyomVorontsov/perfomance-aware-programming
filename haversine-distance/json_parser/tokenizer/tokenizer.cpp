#include "tokenizer.h"

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

    while (isdigit(c) || c == '.')
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
    int i = 0;
    while (*(keyword + i) != '\0')
    {
        if (getc(fileDescriptor) != *(keyword + i))
        {
            fseek(fileDescriptor, -(i + 1), SEEK_CUR);
            return false;
        }
        i++;
    }

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
        if (c == '{')
        {
            addToken(strdup("{"), TOKEN_LBRACE, lexer);
        }
        else if (c == '}')
        {
            addToken(strdup("}"), TOKEN_RBRACE, lexer);
        }
        else if (c == '[')
        {
            addToken(strdup("["), TOKEN_LBRACKET, lexer);
        }
        else if (c == ']')
        {
            addToken(strdup("]"), TOKEN_RBRACKET, lexer);
        }
        else if (c == ',')
        {
            addToken(strdup(","), TOKEN_COMMA, lexer);
        }
        else if (c == ':')
        {
            addToken(strdup(":"), TOKEN_COLON, lexer);
        }
        else if (c == 't')
        {
            ungetc(c, fileDescriptor);

            if (matchKeyword(fileDescriptor, "true", "matchTrue"))
            {
                ungetc(c, fileDescriptor);
                addToken(strdup("true"), TOKEN_TRUE, lexer);
            }
        }
        else if (c == 'f')
        {
            ungetc(c, fileDescriptor);

            if (matchKeyword(fileDescriptor, "false", "matchFalse"))
            {
                ungetc(c, fileDescriptor);
                addToken(strdup("false"), TOKEN_FALSE, lexer);
            }
        }
        else if (c == 'n')
        {
            ungetc(c, fileDescriptor);

            if (matchKeyword(fileDescriptor, "null", "matchNull"))
            {
                ungetc(c, fileDescriptor);
                addToken(strdup("null"), TOKEN_NULL, lexer);
            }
        }
        else if (isspace(c))
        {
            // remove space
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

            }
        }
        else
        {
            c = getc(fileDescriptor);

            printf("Invalid character: %c (%d), location: %ld\n", c, c, ftell(fileDescriptor));
            break;
        }
    }

    // printTokens(lexer);

    return lexer;
}
