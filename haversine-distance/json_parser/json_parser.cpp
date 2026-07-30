#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IS_DEV true
#define ENTRY_AMOUNT 100

typedef struct Entry Entry;
typedef struct Entries Entries;

struct Entries
{
    int count;
    Entry **entries;
};

struct Entry
{
    char *key;
    char *value;
    Entries *link;
    /*
        0 - string value
        1 - int value
        2 - object value
    */
    int type;
};

bool jsonArrayFA(FILE *fd);
bool jsonObjectFA(FILE *fd, Entries *ENTRIES);

// Entry *ENTRIES[ENTRY_AMOUNT];
// int CURRENT_ENTRY = 0;

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
        printf("print: %c", c);
        printDev((char *)"closedBracketFA false\n");

        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}

bool openSquareBracketFA(FILE *fd)
{
    printDev((char *)"openSquareBracketFA\n");
    char c = getc(fd);

    printPart(fd, 1);
    if (c == '[')
    {
        printDev((char *)"openSquareBracketFA true\n");
        return true;
    }
    else
    {
        printDev((char *)"openSquareBracketFA false\n");
        fseek(fd, -1, SEEK_CUR);
        return false;
    }
}

bool closedSquareBracketFA(FILE *fd)
{
    printDev((char *)"closedSquareBracketFA\n");
    char c = getc(fd);

    printPart(fd, 1);

    if (c == ']')
    {
        printDev((char *)"closedSquareBracketFA true\n");

        return true;
    }
    else
    {
        printDev((char *)"closedSquareBracketFA false\n");

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
    char initialFilePointer = ftell(fd);
    char c;
    int i = 0;
    bool isNumber = false;

    c = getc(fd);

    while (c >= '0' && c <= '9')
    {
        c = getc(fd);
        isNumber = true;
        i++;
    }

    if (!isNumber)
    {
        fseek(fd, initialFilePointer, SEEK_SET);
        return false;
    }

    if (c != ',')
    {

        // skip spaces
        int i = 0;
        do
        {
            c = getc(fd);
            i++;
            printf("c %c\n", c);
        } while (c == ' ' || c == '\n' || c == '\t');

        if (c != ',' && c != '}')
        {
            fseek(fd, initialFilePointer, SEEK_SET);
            return false;
        }

        fseek(fd, -i, SEEK_CUR);
    }

    fseek(fd, -1, SEEK_CUR);
    printDev((char *)"numberFA true\n");

    return true;
}

bool keyFA(FILE *fd, Entries *ENTRIES)
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

        ENTRIES->entries[ENTRIES->count] = e;
    }

    printf("keyFA %d\n", res);
    return res;
}

bool valueFA(FILE *fd, Entries *ENTRIES)
{
    int initialStringPtr = ftell(fd);

    printDev((char *)"valueFA\n");
    bool res = stringFA(fd);

    if (res)
    {
        char *str = getStringPart(fd, ftell(fd) - initialStringPtr);
        ENTRIES->entries[ENTRIES->count]->value = str;
        ENTRIES->entries[ENTRIES->count]->type = 0;
    }
    else
    {
        res = numberFA(fd);

        if (res)
        {
            char *str = getStringPart(fd, ftell(fd) - initialStringPtr);
            ENTRIES->entries[ENTRIES->count]->value = str;
            ENTRIES->entries[ENTRIES->count]->type = 1;
        }
        else
        {

            Entry **e = (Entry **)malloc(sizeof(Entry *) * 100);
            Entries *NEW_ENTRIES = (Entries *)malloc(sizeof(Entries));
            NEW_ENTRIES->entries = e;
            NEW_ENTRIES->count = 0;

            res = jsonObjectFA(fd, NEW_ENTRIES);

            // char *str = getStringPart(fd, ftell(fd) - initialStringPtr);
            if (res)
            {
                char *str = (char *)malloc(sizeof(char) * 10);
                str[0] = 'o';
                str[1] = 'b';
                str[2] = 'j';
                str[3] = 'e';
                str[4] = 'c';
                str[5] = 't';
                str[6] = '\0';
                ENTRIES->entries[ENTRIES->count]->link = NEW_ENTRIES;
                ENTRIES->entries[ENTRIES->count]->value = "OBJECT";
                ENTRIES->entries[ENTRIES->count]->type = 2;
            }
        }
    }

    printf("valueFA %d\n", res);
    return res;
}

bool recordFA(FILE *fd, Entries *ENTRIES)
{
    printDev((char *)"recordFA\n");
    bool res = (spaceFA(fd) || 1) &&
               keyFA(fd, ENTRIES) &&
               (spaceFA(fd) || 1) &&
               valueFA(fd, ENTRIES) &&
               (spaceFA(fd) || 1);

    if (res)
    {
        ENTRIES->count++;
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
bool recordsFA(FILE *fd, Entries *ENTRIES)
{
    printDev((char *)"recordsFA\n");

    while (recordFA(fd, ENTRIES) && (spaceFA(fd) || 1) && (comaFA(fd) || 1))
    {
    }

    printf("NUMBER %d\n", ftell(fd));

    return true;
}

bool jsonObjectFA(FILE *fd, Entries *ENTRIES)
{

    printDev((char *)"jsonObjectFA\n");
    printf("jsonObjectFABefore %d\n", ftell(fd));
    bool res = openBracketFA(fd) && recordsFA(fd, ENTRIES) && closedBracketFA(fd);
    printf("jsonObjectFA %d\n", res);
    printf("jsonObjectFA %d\n", ftell(fd));
    return res;
}

bool jsonObjectsFA(FILE *fd)
{
    printDev((char *)"jsonObjectsFA\n");

    Entry **e = (Entry **)malloc(sizeof(Entry *) * 100);
    Entries *NEW_ENTRIES = (Entries *)malloc(sizeof(Entries));
    NEW_ENTRIES->entries = e;
    NEW_ENTRIES->count = 0;

    while (jsonObjectFA(fd, NEW_ENTRIES) && (spaceFA(fd) || 1) && (comaFA(fd)))
    {
        Entry **e = (Entry **)malloc(sizeof(Entry *) * 100);
        Entries *NEW_ENTRIES = (Entries *)malloc(sizeof(Entries));
        NEW_ENTRIES->entries = e;
        NEW_ENTRIES->count = 0;
    }

    return true;
}

bool jsonArrayFA(FILE *fd)
{
    printDev((char *)"jsonArrayFA\n");
    bool res = openSquareBracketFA(fd) && jsonObjectsFA(fd) && closedSquareBracketFA(fd);
    printf("jsonArrayFA %d\n", res);
    return res;
}

void printLevelIndentation(int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("\t");
    }
}

void printEntries(Entries *entries, int level)
{
    for (int i = 0; i < entries->count; i++)
    {

        printLevelIndentation(level);
        printf("key: %s\n", entries->entries[i]->key);
        printLevelIndentation(level);
        printf("value: %s\n", entries->entries[i]->value);
        printLevelIndentation(level);
        printf("link: %s\n", entries->entries[i]->link);
        printLevelIndentation(level);
        printf("type: %d\n", entries->entries[i]->type);
        printf("\n");

        if (entries->entries[i]->type == 2)
        {
            printEntries(entries->entries[i]->link, level + 1);
        }
    }
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

    Entry **e = (Entry **)malloc(sizeof(Entry *) * 100);
    Entries *E = (Entries *)malloc(sizeof(Entries));
    E->entries = e;
    E->count = 0;

    bool res = jsonObjectFA(fd, E);
    printf("isValid: %d\n", res);

    // print entries
    printf("\n");

    printEntries(E, 0);

    printf("Entry amount: %d", E->count);
    printf("\n\n");

    return 0;
}
