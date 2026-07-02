#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>

#include "./shared/sim86_shared.h"

void printMemory(char memory[1024][64], int limit)
{
    int i = 0;
    while (i <= limit)
    {
        printf("%s\n", memory[i]);

        i++;
    }
}

int getOperationCode(char *instruction)
{
    /* operation codes
    mov - 0
    add - 1
    sub - 2
    cmp - 3
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

    if (strcmp(operationName, "add") == 0)
    {
        return 1;
    }

    if (strcmp(operationName, "sub") == 0)
    {
        return 2;
    }

    if (strcmp(operationName, "cmp") == 0)
    {
        return 3;
    }

    return -1;
}

void getRegisterName(int code, char *buffer)
{
    switch (code)
    {

    case 1:
        buffer[0] = 'a';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 2:
        buffer[0] = 'b';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 3:
        buffer[0] = 'c';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 4:
        buffer[0] = 'd';
        buffer[1] = 'x';
        buffer[2] = '\0';
        break;
    case 5:
        buffer[0] = 's';
        buffer[1] = 'p';
        buffer[2] = '\0';
        break;
    case 6:
        buffer[0] = 'b';
        buffer[1] = 'p';
        buffer[2] = '\0';
        break;
    case 7:
        buffer[0] = 's';
        buffer[1] = 'i';
        buffer[2] = '\0';
        break;
    case 8:
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

    char *fileName2 = argv[1];

    if (fileName2 == NULL)
    {
        printf("No file name provided, program terminated.\n");
        return 1;
    }

    FILE *file2 = fopen(fileName2, "r");

    char memory[247][12];
    unsigned char memory2[247] = {0};

    int i = 0;
    while (i < 247)
    {
        memory2[i] = 0x0;
        i++;
    }

    i = 0;
    while (1)
    {
        char c = getc(file2);

        memory2[i] = c;

        if (c == EOF)
        {
            break;
        }

        i++;
    }

    /*
        Register codes
        unused - 0
        ax - 1
        bx - 2
        cx - 3
        dx - 4
        sp - 5
        bp - 6
        si - 7
        di - 8
    */
    uint16_t registers[9];
    // init as zero
    for (int i = 0; i < 9; i++)
    {
        registers[i] = 0x0;
    }

    /*
       Flag register codes
       zf - 0
       sf - 1
       ip - 2
   */
    uint16_t flagRegisters[3];
    // init as zero
    for (int i = 0; i < 3; i++)
    {
        flagRegisters[i] = 0x0;
    }

    int amountOfInstructions = i;

    // Handle operations

    char firstOperand[7];
    char secondOperand[7];

    u32 firstOperandValue;
    u32 secondOperandValue;

    int operation;

    int Offset = 0;
    // Execute instructions
    for (; *(memory2 + Offset) != 0x00;)
    {

        // very stupid way to do simulation, but I messed up from the start
        // so let it be

        instruction Decoded;
        Sim86_Decode8086Instruction(sizeof(memory2) - Offset, (u8 *)memory2 + Offset, &Decoded);
        Offset += Decoded.Size;

        operation = Decoded.Op;

        // Skip unsupported instructios
        // TODO: remove this construction
        if (
            operation != Op_None &&
            operation != Op_mov &&
            operation != Op_add &&
            operation != Op_sub &&
            operation != Op_cmp &&
            operation != Op_jne)
        {
            // printf("Operation %d is not supported", operation);
            continue;
        }

        int ipRegisterPrevValue = flagRegisters[2];
        flagRegisters[2] += Decoded.Size;
        // printf("size: %d\n", Decoded.Size);
        // printf("op: %d\n", Decoded.Op);

        // mov ax, 1 ; ax:0x0->0x1
        // mov bx, 2 ; bx:0x0->0x2
        // mov cx, 3 ; cx:0x0->0x3
        // mov dx, 4 ; dx:0x0->0x4
        // mov sp, 5 ; sp:0x0->0x5
        // mov bp, 6 ; bp:0x0->0x6
        // mov si, 7 ; si:0x0->0x7
        // mov di, 8 ; di:0x0->0x8

        // handle mov operation
        if (operation == Op_mov)
        {

            // Register to register move
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)
            {
                firstOperandValue = Decoded.Operands[0].Register.Index;
                secondOperandValue = Decoded.Operands[1].Register.Index;

                char firstRegisterNameBuffer[3];
                char secondRegisterNameBuffer[3];
                getRegisterName(Decoded.Operands[0].Register.Index, firstRegisterNameBuffer);
                getRegisterName(Decoded.Operands[1].Register.Index, secondRegisterNameBuffer);

                printf("mov %s, %s ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondRegisterNameBuffer,
                       firstRegisterNameBuffer,
                       registers[Decoded.Operands[0].Register.Index],
                       registers[Decoded.Operands[1].Register.Index]);

                registers[Decoded.Operands[0].Register.Index] = registers[Decoded.Operands[1].Register.Index];
            }

            // Immidiate value to register move
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                firstOperandValue = Decoded.Operands[0].Register.Index;
                secondOperandValue = Decoded.Operands[1].Register.Index;
                char firstRegisterNameBuffer[3];
                getRegisterName(Decoded.Operands[0].Register.Index, firstRegisterNameBuffer);

                printf("mov %s, %d ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondOperandValue,
                       firstRegisterNameBuffer,
                       registers[Decoded.Operands[0].Register.Index],
                       secondOperandValue);

                registers[Decoded.Operands[0].Register.Index] = secondOperandValue;
            }

            printf("ip:0x%x->0x%x ", ipRegisterPrevValue, flagRegisters[2]);
            printf("\n");
        }

        // handle add operation
        if (operation == Op_add)
        {
            uint16_t flagRegistersInitialState[2];
            flagRegistersInitialState[0] = flagRegisters[0];
            flagRegistersInitialState[1] = flagRegisters[1];

            firstOperandValue = Decoded.Operands[0].Register.Index;
            secondOperandValue = Decoded.Operands[1].Register.Index;

            char firstRegisterNameBuffer[3];
            char secondRegisterNameBuffer[3];
            getRegisterName(firstOperandValue, firstRegisterNameBuffer);
            getRegisterName(secondOperandValue, secondRegisterNameBuffer);

            // Register to register add
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)
            {
                printf("add %s %s; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondRegisterNameBuffer,
                       firstRegisterNameBuffer,
                       registers[firstOperandValue],
                       registers[firstOperandValue] + registers[secondOperandValue]);
                registers[firstOperandValue] = registers[firstOperandValue] + registers[secondOperandValue];

                flagRegisters[0] = (registers[firstOperandValue]) == 0;
                flagRegisters[1] = !!(registers[firstOperandValue] & 0b1000000000000000);
            }

            // Immidiate value to register add
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                printf("add %s, %d ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondOperandValue,
                       firstRegisterNameBuffer,
                       registers[firstOperandValue],
                       registers[firstOperandValue] + secondOperandValue);
                registers[firstOperandValue] = registers[firstOperandValue] + secondOperandValue;

                flagRegisters[0] = (registers[firstOperandValue]) == 0;
                flagRegisters[1] = !!(registers[firstOperandValue] & 0b1000000000000000);
            }

            printf("ip:0x%x->0x%x ", ipRegisterPrevValue, flagRegisters[2]);

            if (flagRegisters[0] || flagRegisters[1] || flagRegistersInitialState[0] || flagRegistersInitialState[1])
            {
                printf("flags:");

                if (flagRegistersInitialState[0])
                {
                    printf("Z");
                }

                if (flagRegistersInitialState[1])
                {
                    printf("S");
                }

                printf("->");

                if (flagRegisters[0])
                {
                    printf("Z");
                }

                if (flagRegisters[1])
                {
                    printf("S");
                }
                printf(" ");
            }

            printf("\n");
        }

        // handle sub operation
        if (operation == Op_sub)
        {
            uint16_t flagRegistersInitialState[2];
            flagRegistersInitialState[0] = flagRegisters[0];
            flagRegistersInitialState[1] = flagRegisters[1];

            firstOperandValue = Decoded.Operands[0].Register.Index;
            secondOperandValue = Decoded.Operands[1].Register.Index;

            char firstRegisterNameBuffer[3];
            char secondRegisterNameBuffer[3];
            getRegisterName(firstOperandValue, firstRegisterNameBuffer);
            getRegisterName(secondOperandValue, secondRegisterNameBuffer);

            // Register to register sub
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)
            {
                printf("sub %s, %s ; %s:0x%03x->0x%03x ",
                       firstRegisterNameBuffer,
                       secondRegisterNameBuffer,
                       firstRegisterNameBuffer,
                       registers[firstOperandValue],
                       (uint16_t)(registers[firstOperandValue] - registers[secondOperandValue]));
                registers[firstOperandValue] = registers[firstOperandValue] - registers[secondOperandValue];

                flagRegisters[0] = (registers[firstOperandValue]) == 0;
                flagRegisters[1] = !!(registers[firstOperandValue] & 0b1000000000000000);
            }

            // Immidiate value to register sub
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                printf("sub %s, %d ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondOperandValue,
                       firstRegisterNameBuffer,
                       registers[firstOperandValue],
                       registers[firstOperandValue] - secondOperandValue);
                registers[firstOperandValue] = registers[firstOperandValue] - secondOperandValue;

                flagRegisters[0] = (registers[firstOperandValue]) == 0;
                flagRegisters[1] = !!(registers[firstOperandValue] & 0b1000000000000000);
            }

            printf("ip:0x%x->0x%x ", ipRegisterPrevValue, flagRegisters[2]);

            if (flagRegisters[0] || flagRegisters[1] || flagRegistersInitialState[0] || flagRegistersInitialState[1])
            {
                printf("flags:");

                if (flagRegistersInitialState[0])
                {
                    printf("Z");
                }

                if (flagRegistersInitialState[1])
                {
                    printf("S");
                }

                printf("->");

                if (flagRegisters[0])
                {
                    printf("Z");
                }

                if (flagRegisters[1])
                {
                    printf("S");
                }
                printf(" ");
            }

            printf("\n");
        }

        // handle cmp operation
        if (operation == Op_cmp)
        {
            uint16_t flagRegistersInitialState[2];
            flagRegistersInitialState[0] = flagRegisters[0];
            flagRegistersInitialState[1] = flagRegisters[1];

            firstOperandValue = Decoded.Operands[0].Register.Index;
            secondOperandValue = Decoded.Operands[1].Register.Index;

            char firstRegisterNameBuffer[3];
            char secondRegisterNameBuffer[3];
            getRegisterName(firstOperandValue, firstRegisterNameBuffer);
            getRegisterName(secondOperandValue, secondRegisterNameBuffer);

            // Register to register cmp
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)

            {
                printf("cmp %s, %s ; ", firstRegisterNameBuffer, secondRegisterNameBuffer);

                flagRegisters[0] = (registers[firstOperandValue]) == 0;
                flagRegisters[1] = !!(registers[firstOperandValue] & 0b1000000000000000);
            }

            // Immidiate value to register cmp
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                printf("cmp %s %d; ", firstRegisterNameBuffer, secondOperandValue);

                flagRegisters[0] = (registers[firstOperandValue] - secondOperandValue) == 0;
                flagRegisters[1] = !!(registers[firstOperandValue] & 0b1000000000000000);
            }

            if (flagRegisters[0] || flagRegisters[1] || flagRegistersInitialState[0] || flagRegistersInitialState[1])
            {
                printf("flags:");

                if (flagRegistersInitialState[0])
                {
                    printf("Z");
                }

                if (flagRegistersInitialState[1])
                {
                    printf("S");
                }

                printf("->");

                if (flagRegisters[0])
                {
                    printf("Z");
                }

                if (flagRegisters[1])
                {
                    printf("S");
                }
                printf(" ");
            }
            printf("\n");
        }

        // handle jne operation
        if (operation == Op_jne)
        {

            int16_t fov = Decoded.Operands[0].Register.Index;

            printf("jne $%d ", fov + Decoded.Size);

            if (flagRegisters[0] == 0)
            {
                printf("; ip:0x%x->0x%x \n", ipRegisterPrevValue, flagRegisters[2] + fov);
                flagRegisters[2] = flagRegisters[2] + fov;
                Offset = flagRegisters[2];
            } else {
                printf("; ip:0x%x->0x%x \n", ipRegisterPrevValue, flagRegisters[2]);
            }
        }
    }
    printf("\n");

    // print registers
    printf("Final registers:\n");

    for (int i = 0; i < 9; i++)
    {
        if (registers[i])
        {
            printf("      ");
            char buffer[3];
            getRegisterName(i, buffer);
            printf("%s", buffer);

            printf(": ");

            printf("0x%.4x (%d)\n", registers[i], registers[i]);
        }
    }

    printf("      ip: 0x%.4x (%d)\n", flagRegisters[2], flagRegisters[2]);

    printf("   flags: ");
    if (flagRegisters[0])
    {
        printf("Z");
    }

    if (flagRegisters[1])
    {
        printf("S");
    }

    return 0;
}
