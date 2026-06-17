#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

void printMemory(char memory[1024][64], int limit)
{
    int i = 0;
    while (i <= limit)
    {
        printf("%s\n", memory[i]);

        i++;
    }
}

int atoi(char *c)
{
    char a[10];
    strcpy(a, c);
    int i = 0;
    int numberLength = 0;
    int negativeMultiplier = 1;
    int number = 0;

    // check for negative number
    if (a[0] == '-')
    {
        negativeMultiplier = -1;

        int i = 1;
        while (1)
        {
            if (a[i] == '\0')
            {
                break;
            }
            a[i - 1] = a[i];
            i++;
        }
        a[i - 1] = '\0';
    }

    // get number length
    i = 0;
    while (1)
    {
        if (a[i] == '\0')
        {
            break;
        }

        numberLength++;
        i++;
    }

    // transform ascii to int
    int j = numberLength;

    i = 0;
    while (j != 0)
    {
        number += (a[j - 1] - '0') * (int)pow(10, i);

        j--;
        i++;
    }

    return number * negativeMultiplier;
}

int getOperationCode(char *instruction)
{
    /* operation codes
    mov - 0
    */
    char operationName[4];
    operationName[3] = '\0';

    int i = 0;
    while (1)
    {

        if (instruction[i] == ' ')
        {
            break;
        }

        operationName[i] = instruction[i];

        i++;
    }

    if (strcmp(operationName, "mov") == 0)
    {
        return 0;
    }

    return -1;
}

void getFirstOperand(char *instruction, char *firstOperand)
{

    int i = 0;
    while (1)
    {
        i++;
        if (instruction[i] == ' ')
        {
            break;
        }
    }

    i++;

    int j = 0;
    while (1)
    {

        if (instruction[i] == ',')
        {
            break;
        }

        firstOperand[j] = instruction[i];

        j++;
        i++;
    }
    j++;

    firstOperand = '\0';
}

void getSecondOperand(char *instruction, char *secondOperand)
{

    int i = 0;
    bool firstOperandSkipped = false;
    while (1)
    {
        i++;
        if (instruction[i] == ' ')
        {

            if (firstOperandSkipped)
            {
                break;
            }

            firstOperandSkipped = true;
        }
    }
    i++;

    int j = 0;
    while (1)
    {

        if (instruction[i] == '\0')
        {
            break;
        }

        secondOperand[j] = instruction[i];

        j++;
        i++;
    }

    j++;
    secondOperand[j] = '\0';
}

int getRegisterCode(char *operand)
{
    int operandCode = -1;

    if (strcmp(operand, "ax") == 0)
    {
        operandCode = 0;
    }
    else if (strcmp(operand, "bx") == 0)
    {
        operandCode = 1;
    }
    else if (strcmp(operand, "cx") == 0)
    {
        operandCode = 2;
    }
    else if (strcmp(operand, "dx") == 0)
    {
        operandCode = 3;
    }
    else if (strcmp(operand, "sp") == 0)
    {
        operandCode = 4;
    }
    else if (strcmp(operand, "bp") == 0)
    {
        operandCode = 5;
    }
    else if (strcmp(operand, "si") == 0)
    {
        operandCode = 6;
    }
    else if (strcmp(operand, "di") == 0)
    {
        operandCode = 7;
    }

    return operandCode;
}

void getRegisterName(int code, char *buffer)
{
    switch (code)
    {

    case 0:
        buffer[0] = 'a';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 1:
        buffer[0] = 'b';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 2:
        buffer[0] = 'c';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 3:
        buffer[0] = 'd';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 4:
        buffer[0] = 's';
        buffer[1] = 'p';
        buffer[2] = '\0';
        break;
    case 5:
        buffer[0] = 'b';
        buffer[1] = 'p';
        buffer[2] = '\0';
        break;
    case 6:
        buffer[0] = 's';
        buffer[1] = 'i';
        buffer[2] = '\0';
        break;
    case 7:
        buffer[0] = 'd';
        buffer[1] = 'i';
        buffer[2] = '\0';
        break;

    default:
        break;
    }
}

int main(int argc, char *argv[])
{

    char *fileName = argv[1];

    if (fileName == NULL)
    {
        printf("No file name provided, program terminated.\n");
        return 1;
    }

    FILE *file = fopen(fileName, "r");

    char memory[1024][64];

    /*
        Register codes
        ax - 0
        bx - 1
        cx - 2
        dx - 3
        sp - 4
        bp - 5
        si - 6
        di - 7
    */
    int registers[8];

    // read instructions into memory
    int i = 0;
    while (1)
    {

        bool endOfFile = false;
        bool emptyLine = false;

        // read and store instruction
        int j = 0;
        while (1)
        {
            char c = getc(file);

            if (c == '\n' || c == EOF)
            {

                endOfFile = c == EOF;
                emptyLine = j == 0;

                break;
            }

            memory[i][j] = c;

            j++;
        }
        j++;

        memory[i][j] = '\0';

        if (endOfFile)
        {
            break;
        }

        if (emptyLine == false)
        {
            i++;
        }
    }

    int amountOfInstructions = i;

    // Handle operations

    char firstOperand[4];
    char secondOperand[4];
    int operation;

    for (int i = 0; i <= amountOfInstructions; i++)
    {
        operation = getOperationCode(memory[i]);

        // mov ax, 1 ; ax:0x0->0x1
        // mov bx, 2 ; bx:0x0->0x2
        // mov cx, 3 ; cx:0x0->0x3
        // mov dx, 4 ; dx:0x0->0x4
        // mov sp, 5 ; sp:0x0->0x5
        // mov bp, 6 ; bp:0x0->0x6
        // mov si, 7 ; si:0x0->0x7
        // mov di, 8 ; di:0x0->0x8

        // handle mov operation
        if (operation == 0)
        {

            getFirstOperand(memory[i], firstOperand);
            getSecondOperand(memory[i], secondOperand);

            int registerCodeForFirstOperand = getRegisterCode(firstOperand);
            int registerCodeForSecondOperand = getRegisterCode(secondOperand);

            // Register to register move
            if (registerCodeForFirstOperand != -1 && registerCodeForSecondOperand != -1)
            {
                printf("%s ; %s:0x%X->0x%X \n", memory[i], firstOperand, registers[registerCodeForFirstOperand], registers[registerCodeForSecondOperand]);
                registers[registerCodeForFirstOperand] = registers[registerCodeForSecondOperand];
            }

            // Immidiate value to register move
            if (registerCodeForFirstOperand != -1 && registerCodeForSecondOperand == -1)
            {
                printf("%s ; %s:0x%X->0x%X \n", memory[i], firstOperand, registers[registerCodeForFirstOperand], atoi(secondOperand));
                registers[registerCodeForFirstOperand] = atoi(secondOperand);
            }
        }
    }
    printf("\n");

    // print registers
    printf("Final registers:\n");

    for (int i = 0; i < 8; i++)
    {
        printf("    ");
        char buffer[3];
        getRegisterName(i, buffer);
        printf("%s", buffer);

        printf(": ");

        printf("0x%.4X (%d)\n", registers[i], registers[i]);
    }

    return 0;
}
