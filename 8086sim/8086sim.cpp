#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>

#include "./shared/sim86_shared.h"

void getRegisterName(int code, char *buffer);
void printFlagChanges(uint16_t flagRegisters[3], uint16_t flagRegistersInitialState[3]);
void printIpRegisterChanges(uint16_t ipRegisterCurrentValue, uint16_t ipRegisterPrevValue);
void printRegistersFinalState(uint16_t registers[9], uint16_t flagRegisters[3]);

int main(int argc, char *argv[])
{

    char *fileName = argv[1];

    if (fileName == NULL)
    {
        printf("No file name provided, program terminated.\n");
        return 1;
    }

    FILE *file = fopen(fileName, "r");
    if (!file)
    {
        printf("Issue while reading the file");
        return 1;
    }

    unsigned char memory[64 * 1024] = {0};

    int i = 0;

    // Load file into memory
    while (1)
    {
        int c = getc(file);

        if (c == EOF)
        {
            break;
        }

        memory[i] = c;

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
    uint16_t registers[9] = {0};

    /*
       Flag register codes
       zf - 0
       sf - 1
       ip - 2
   */
    uint16_t flagRegisters[3] = {0};

    uint32_t firstOperandRegisterIndex;
    uint32_t secondOperandRegisterIndex;
    int Offset = 0;

    // Execute instructions
    for (; *(memory + Offset) != 0x00;)
    {

        instruction Decoded;
        Sim86_Decode8086Instruction(sizeof(memory) - Offset, (u8 *)memory + Offset, &Decoded);
        Offset += Decoded.Size;

        int operation = Decoded.Op;
        int ipRegisterPrevValue = flagRegisters[2];
        flagRegisters[2] += Decoded.Size;
        char firstRegisterNameBuffer[3] = {0};
        char secondRegisterNameBuffer[3] = {0};

        // handle mov operation
        if (operation == Op_mov)
        {

            // Register to register move
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)
            {
                firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
                secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;

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
                firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
                secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;
                getRegisterName(Decoded.Operands[0].Register.Index, firstRegisterNameBuffer);

                printf("mov %s, %d ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondOperandRegisterIndex,
                       firstRegisterNameBuffer,
                       registers[Decoded.Operands[0].Register.Index],
                       secondOperandRegisterIndex);

                registers[Decoded.Operands[0].Register.Index] = secondOperandRegisterIndex;
            }

            // Store immidiate value into memory
            if (Decoded.Operands[0].Type == Operand_Memory && Decoded.Operands[1].Type == Operand_Immediate)
            {

                uint32_t firstOperandAddressDisplacement = Decoded.Operands[0].Address.Displacement;
                firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
                getRegisterName(firstOperandRegisterIndex, firstRegisterNameBuffer);

                secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;

                if (firstOperandRegisterIndex)
                {
                    printf("mov word [%s+%d], %d ; ", firstRegisterNameBuffer, firstOperandAddressDisplacement, secondOperandRegisterIndex);
                    memory[firstOperandAddressDisplacement + registers[firstOperandRegisterIndex]] = secondOperandRegisterIndex;
                }
                else
                {
                    printf("mov word [+%d], %d ; ", firstOperandAddressDisplacement, secondOperandRegisterIndex);
                    memory[firstOperandAddressDisplacement] = secondOperandRegisterIndex;
                }
            }

            // Load value into register from memory
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Memory)
            {

                uint32_t secondOperandAddressDisplacement = Decoded.Operands[1].Address.Displacement;
                firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
                getRegisterName(firstOperandRegisterIndex, firstRegisterNameBuffer);

                secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;
                getRegisterName(secondOperandRegisterIndex, secondRegisterNameBuffer);

                if (secondOperandRegisterIndex)
                {
                    printf("mov %s, [%s+%d] ; %s:0x%x->0x%x ",
                           firstRegisterNameBuffer,
                           secondRegisterNameBuffer,
                           secondOperandAddressDisplacement,
                           firstRegisterNameBuffer,
                           registers[firstOperandRegisterIndex],
                           memory[registers[secondOperandRegisterIndex] + secondOperandAddressDisplacement]);
                    registers[firstOperandRegisterIndex] = memory[registers[secondOperandRegisterIndex] + secondOperandAddressDisplacement];
                }
                else
                {
                    printf("mov %s, [+%d] ; %s:0x%x->0x%x ",
                           firstRegisterNameBuffer,
                           secondOperandAddressDisplacement,
                           firstRegisterNameBuffer,
                           registers[firstOperandRegisterIndex],
                           memory[secondOperandAddressDisplacement]);
                    registers[firstOperandRegisterIndex] = memory[secondOperandAddressDisplacement];
                }
            }

            printIpRegisterChanges(flagRegisters[2], ipRegisterPrevValue);
            printf("\n");
        }

        // handle add operation
        if (operation == Op_add)
        {
            uint16_t flagRegistersInitialState[2];
            flagRegistersInitialState[0] = flagRegisters[0];
            flagRegistersInitialState[1] = flagRegisters[1];

            firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
            secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;

            getRegisterName(firstOperandRegisterIndex, firstRegisterNameBuffer);
            getRegisterName(secondOperandRegisterIndex, secondRegisterNameBuffer);

            // Register to register add
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)
            {
                printf("add %s %s; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondRegisterNameBuffer,
                       firstRegisterNameBuffer,
                       registers[firstOperandRegisterIndex],
                       registers[firstOperandRegisterIndex] + registers[secondOperandRegisterIndex]);
                registers[firstOperandRegisterIndex] = registers[firstOperandRegisterIndex] + registers[secondOperandRegisterIndex];

                flagRegisters[0] = (registers[firstOperandRegisterIndex]) == 0;
                flagRegisters[1] = !!(registers[firstOperandRegisterIndex] & 0b1000000000000000);
            }

            // Immidiate value to register add
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                printf("add %s, %d ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondOperandRegisterIndex,
                       firstRegisterNameBuffer,
                       registers[firstOperandRegisterIndex],
                       registers[firstOperandRegisterIndex] + secondOperandRegisterIndex);
                registers[firstOperandRegisterIndex] = registers[firstOperandRegisterIndex] + secondOperandRegisterIndex;

                flagRegisters[0] = (registers[firstOperandRegisterIndex]) == 0;
                flagRegisters[1] = !!(registers[firstOperandRegisterIndex] & 0b1000000000000000);
            }

            printIpRegisterChanges(flagRegisters[2], ipRegisterPrevValue);
            printFlagChanges(flagRegisters, flagRegistersInitialState);

            printf("\n");
        }

        // handle sub operation
        if (operation == Op_sub)
        {
            uint16_t flagRegistersInitialState[2];
            flagRegistersInitialState[0] = flagRegisters[0];
            flagRegistersInitialState[1] = flagRegisters[1];

            firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
            secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;

            getRegisterName(firstOperandRegisterIndex, firstRegisterNameBuffer);
            getRegisterName(secondOperandRegisterIndex, secondRegisterNameBuffer);

            // Register to register sub
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)
            {
                printf("sub %s, %s ; %s:0x%03x->0x%03x ",
                       firstRegisterNameBuffer,
                       secondRegisterNameBuffer,
                       firstRegisterNameBuffer,
                       registers[firstOperandRegisterIndex],
                       (uint16_t)(registers[firstOperandRegisterIndex] - registers[secondOperandRegisterIndex]));
                registers[firstOperandRegisterIndex] = registers[firstOperandRegisterIndex] - registers[secondOperandRegisterIndex];

                flagRegisters[0] = (registers[firstOperandRegisterIndex]) == 0;
                flagRegisters[1] = !!(registers[firstOperandRegisterIndex] & 0b1000000000000000);
            }

            // Immidiate value to register sub
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                printf("sub %s, %d ; %s:0x%x->0x%x ",
                       firstRegisterNameBuffer,
                       secondOperandRegisterIndex,
                       firstRegisterNameBuffer,
                       registers[firstOperandRegisterIndex],
                       registers[firstOperandRegisterIndex] - secondOperandRegisterIndex);
                registers[firstOperandRegisterIndex] = registers[firstOperandRegisterIndex] - secondOperandRegisterIndex;

                flagRegisters[0] = (registers[firstOperandRegisterIndex]) == 0;
                flagRegisters[1] = !!(registers[firstOperandRegisterIndex] & 0b1000000000000000);
            }

            printIpRegisterChanges(flagRegisters[2], ipRegisterPrevValue);
            printFlagChanges(flagRegisters, flagRegistersInitialState);

            printf("\n");
        }

        // handle cmp operation
        if (operation == Op_cmp)
        {
            uint16_t flagRegistersInitialState[2];
            flagRegistersInitialState[0] = flagRegisters[0];
            flagRegistersInitialState[1] = flagRegisters[1];

            firstOperandRegisterIndex = Decoded.Operands[0].Register.Index;
            secondOperandRegisterIndex = Decoded.Operands[1].Register.Index;

            getRegisterName(firstOperandRegisterIndex, firstRegisterNameBuffer);
            getRegisterName(secondOperandRegisterIndex, secondRegisterNameBuffer);

            // Register to register cmp
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Register)

            {
                printf("cmp %s, %s ; ", firstRegisterNameBuffer, secondRegisterNameBuffer);

                flagRegisters[0] = (registers[firstOperandRegisterIndex]) == 0;
                flagRegisters[1] = !!(registers[firstOperandRegisterIndex] & 0b1000000000000000);
            }

            // Immidiate value to register cmp
            if (Decoded.Operands[0].Type == Operand_Register && Decoded.Operands[1].Type == Operand_Immediate)
            {
                printf("cmp %s %d; ", firstRegisterNameBuffer, secondOperandRegisterIndex);

                flagRegisters[0] = (registers[firstOperandRegisterIndex] - secondOperandRegisterIndex) == 0;
                flagRegisters[1] = !!(registers[firstOperandRegisterIndex] & 0b1000000000000000);
            }

            printIpRegisterChanges(flagRegisters[2], ipRegisterPrevValue);
            printFlagChanges(flagRegisters, flagRegistersInitialState);

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
            }
            else
            {
                printf("; ip:0x%x->0x%x \n", ipRegisterPrevValue, flagRegisters[2]);
            }
        }
    }
    printf("\n");
    printRegistersFinalState(registers, flagRegisters);

    return 0;
}

void printFlagChanges(uint16_t flagRegisters[3], uint16_t flagRegistersInitialState[3])
{
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
}

void printIpRegisterChanges(uint16_t ipRegisterCurrentValue, uint16_t ipRegisterPrevValue)
{
    printf("ip:0x%x->0x%x ", ipRegisterPrevValue, ipRegisterCurrentValue);
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

void printRegistersFinalState(uint16_t registers[9], uint16_t flagRegisters[3])
{
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
}