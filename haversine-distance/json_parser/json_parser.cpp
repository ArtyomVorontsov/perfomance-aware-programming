#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IS_DEV true
#define ENTRY_AMOUNT 100

struct Entry
{
    char *key;
    char *value;
    /*
        0 - string value
        1 - int value
    */
    int type;
};

Entry *ENTRIES[ENTRY_AMOUNT];
int CURRENT_ENTRY = 0;

void printDev(char *msg)
{
    if (IS_DEV)
        printf((const char *)msg);
}

void printPart(FILE *fd, int end)
{
    fseek(fd, -end, SEEK_CUR);
    char c;
    while (end--)
    {
        c = getc(fd);
    }
}

char *getStringPart(FILE *fd, int end)
{
    printDev((char *)"getStringPart\n");
    fseek(fd, -end, SEEK_CUR);

    char c;
    int i = 0;
    char *buffer = (char *)malloc(sizeof(char) * (end + 1));

    while (end--)
    {
        c = getc(fd);
        buffer[i] = c;
        i++;
    }

    buffer[i] = '\0';

    return buffer;
}

bool openBracketFA(FILE *fd)
{
    printDev((char *)"openBracketFA\n");
    char c = getc(fd);

    printPart(fd, 1);
    if (c == '{')
    {
        printDev((char *)"openBracketFA true\n");
        return true;
    }
    else
    {
        printDev((char *)"openBracketFA false\n");
        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}

bool closedBracketFA(FILE *fd)
{
    printDev((char *)"closedBracketFA\n");
    char c = getc(fd);

    printPart(fd, 1);

    if (c == '}')
    {
        printDev((char *)"closedBracketFA true\n");

        return true;
    }
    else
    {
        printDev((char *)"closedBracketFA false\n");

        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}

bool doubleQuoteFA(FILE *fd)
{
    printDev((char *)"doubleQuoteFA\n");
    char c = getc(fd);

    printPart(fd, 1);

    if (c == '"')
    {
        printDev((char *)"doubleQuoteFA true\n");

        return true;
    }
    else
    {
        printDev((char *)"doubleQuoteFA false\n");

        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}

bool colonFA(FILE *fd)
{
    printDev((char *)"colonFA\n");
    char c = getc(fd);

    printPart(fd, 1);

    if (c == ':')
    {
        printDev((char *)"colonFA true\n");

        return true;
    }
    else
    {
        printDev((char *)"colonFA false\n");

        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}

bool spaceFA(FILE *fd)
{
    printDev((char *)"spaceFA\n");

    char c;
    int i = 0;
    do
    {
        c = getc(fd);

        if (c != ' ' && c != '\t' && c != '\n')
        {
            fseek(fd, -1, SEEK_CUR);
            printDev((char *)"spaceFA true\n");

            return true;
        }

        i++;

    } while (c == ' ' || c == '\t' || c == '\n');

    fseek(fd, -1, SEEK_CUR);
    return false;
}

bool charsFA(FILE *fd)
{
    printDev((char *)"charsFA\n");

    char c;
    int i = 0;
    do
    {
        c = getc(fd);

        if (c == '"')
        {
            fseek(fd, -1, SEEK_CUR);
            printDev((char *)"charsFA true\n");

            return true;
        }

        i++;

    } while (c >= 0 && c <= 127);

    fseek(fd, -i, SEEK_CUR);

    printDev((char *)"charsFA false\n");

    return false;
}

bool stringFA(FILE *fd)
{
    printDev((char *)"stringFA\n");
    char initialFilePointer = ftell(fd);
    if (doubleQuoteFA(fd) && charsFA(fd) && doubleQuoteFA(fd))
    {
        printDev((char *)"stringFA true\n");

        return true;
    }
    else
    {
        fseek(fd, initialFilePointer, SEEK_SET);

        printDev((char *)"stringFA false\n");

        return false;
    }
}

bool numberFA(FILE *fd)
{
    printDev((char *)"numberFA\n");

    char c;
    int i = 0;
    do
    {
        c = getc(fd);

        i++;

    } while (c >= '0' && c <= '9');

    fseek(fd, -1, SEEK_CUR);
    printDev((char *)"numberFA true\n");

    return true;
}

bool keyFA(FILE *fd)
{
    printDev((char *)"keyFA\n");
    int initialStringPtr;
    int endStringPtr;
    bool res = (spaceFA(fd) || 1) &&
               ((initialStringPtr = ftell(fd)) || 1) &&
               stringFA(fd) &&
               ((endStringPtr = ftell(fd)) || 1) &&
               (spaceFA(fd) || 1) &&
               colonFA(fd);

    if (res)
    {
        int currentStringPtr = ftell(fd);
        fseek(fd, endStringPtr, SEEK_SET);
        char *str = getStringPart(fd, endStringPtr - initialStringPtr);
        fseek(fd, currentStringPtr, SEEK_SET);

        Entry *e = (Entry *)malloc(sizeof(Entry));
        e->key = str;

        ENTRIES[CURRENT_ENTRY] = e;
    }

    printf("keyFA %d\n", res);
    return res;
}

bool valueFA(FILE *fd)
{
    int initialStringPtr = ftell(fd);

    printDev((char *)"valueFA\n");
    bool res = stringFA(fd);

    if (res)
    {
        char *str = getStringPart(fd, ftell(fd) - initialStringPtr);
        ENTRIES[CURRENT_ENTRY]->value = str;
        ENTRIES[CURRENT_ENTRY]->type = 0;
    }
    else
    {
        res = numberFA(fd);

        if (res)
        {
            char *str = getStringPart(fd, ftell(fd) - initialStringPtr);
            ENTRIES[CURRENT_ENTRY]->value = str;
            ENTRIES[CURRENT_ENTRY]->type = 1;
        }
    }

    printf("valueFA %d\n", res);
    return res;
}

bool recordFA(FILE *fd)
{
    printDev((char *)"recordFA\n");
    bool res = (spaceFA(fd) || 1) &&
               keyFA(fd) &&
               (spaceFA(fd) || 1) &&
               valueFA(fd) &&
               (spaceFA(fd) || 1);

    if (res)
    {
        CURRENT_ENTRY++;
    }

    printf("recordFA %d\n", res);
    return res;
}

bool comaFA(FILE *fd)
{
    printDev((char *)"comaFA\n");
    char c = getc(fd);

    printPart(fd, 1);

    if (c == ',')
    {
        printDev((char *)"comaFA true\n");

        return true;
    }
    else
    {
        printDev((char *)"comaFA false\n");

        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}
bool recordsFA(FILE *fd)
{
    printDev((char *)"recordsFA\n");

    while (recordFA(fd) && (spaceFA(fd) || 1) && (comaFA(fd)))
    {
    }

    return true;
}

bool jsonFA(FILE *fd)
{
    printDev((char *)"jsonFA\n");
    bool res = openBracketFA(fd) && recordsFA(fd) && closedBracketFA(fd);
    printf("jsonFA %d\n", res);
    return res;
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printDev((char *)"No json file provided.");
        return 1;
    }

    char *fileName = argv[1];

    FILE *fd = fopen(fileName, "r");

    char buffer[1024];

    char c;
    bool insideObject = false;

    bool res = jsonFA(fd);
    printf("isValid: %d\n", res);

    // print entries
    printf("\n");

    printf("Entry amount: %d", CURRENT_ENTRY);
    printf("\n\n");

    for (int i = 0; i < CURRENT_ENTRY; i++)
    {
        printf("key: %s\n", ENTRIES[i]->key);
        printf("value: %s\n", ENTRIES[i]->value);
        printf("type: %d\n", ENTRIES[i]->type);
        printf("=================== \n");
    }

    return 0;
}
