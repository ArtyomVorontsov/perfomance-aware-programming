#include <stdio.h>
#include <string.h> /* strcat */
#include <stdlib.h> /* strtol */
#include <stdint.h>

/* 8086 simulator */
/*
 * TODO:
 * Read binary file - done
 * Disassembly file
 * Consider R/M, D, MOD, DISP-LO, DISP-HI fields
 * Support different instructions, such as immidiate, add and sub
 */

const char *byte_to_binary(int x)
{
	static char b[9];
	b[0] = '\0';

	int z;
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}

char loadInstructionByte(FILE *file, char *buffer, int *i)
{
	char b = getc(file);
	buffer[*i++] = b;

	return b;
}

void getBaseName(char *name)
{

	int i = 1;

	for (; *(name + i) != '.' && *(name + i) != '\0'; i++)
		;

	name[i] = '\0';
}

int main(int argc, char *argv[])
{

	char *fileName = argv[1];
	char outFileName[100];
	printf("File name: %s\n", fileName);
	strcpy(outFileName, argv[1]);
	getBaseName(outFileName);
	strcat(outFileName, ".out");
	printf("outFileName: %s\n", outFileName);

	FILE *file = fopen(fileName, "r");
	FILE *fileOut = fopen(outFileName, "w");

	unsigned char buffer[1024] = {'\n'};
	char byte1, byte2, byte3, byte4, byte5, byte6;
	int i = 0;

	char *opcodes[64],
		ds[2],
		ws[2],
		mods[4],
		*regs[8],
		rms[8];

	opcodes[34] = "mov";
	opcodes[11] = "mov";

	unsigned int opcode = 0,
				 d = 0,
				 w = 0,
				 mod = 0,
				 reg = 0,
				 rm = 0,
				 data1 = 0,
				 data2 = 0;

	uint64_t opcodeMask = 0b11111111,
			 dMask = 0,
			 wMask = 0,
			 modMask = 0,
			 regMask = 0,
			 rmMask = 0,
			 data1Mask = 0,
			 data2Mask = 0;

	fprintf(fileOut, "bits 16\n");

	uint64_t instructions[2][2][13] = {
		// MOV

		// register/memory to/from register
		{
			// opcode
			{0b100010},
			{
				// opcode mask - 0
				0b100010000000000000000000000000000000000000000000,
				// d mask - 1
				0b000000100000000000000000000000000000000000000000,
				// w mask - 2
				0b000000010000000000000000000000000000000000000000,
				// mod mask - 3
				0b000000001100000000000000000000000000000000000000,
				// reg mask - 4
				0b000000000011100000000000000000000000000000000000,
				// r/m mask - 5
				0b000000000000011100000000000000000000000000000000,
				// disp-lo mask - 6
				0b000000000000000011111111000000000000000000000000,
				// disp-hi mask - 7
				0b000000000000000000000000111111110000000000000000,
				// data1 mask - 8
				0b000000000000000000000000000000000000000000000000,
				// data2 mask - 9
				0b000000000000000000000000000000000000000000000000,
				// SR mask - 10
				0b000000000000000000000000000000000000000000000000,
				// addr-lo mask - 11
				0b000000000000000000000000000000000000000000000000,
				// addr-hi mask - 12
				0b000000000000000000000000000000000000000000000000,

			},
		},
		// immidiate to register
		{
			// opcode
			{0b1011},
			{
				// opcode mask - 0
				0b101100000000000000000000000000000000000000000000,
				// d mask - 1
				0b000000000000000000000000000000000000000000000000,
				// w mask - 2
				0b000010000000000000000000000000000000000000000000,
				// mod mask - 3
				0b000000000000000000000000000000000000000000000000,
				// reg mask - 4
				0b000001110000000000000000000000000000000000000000,
				// r/m mask - 5
				0b000000000000000000000000000000000000000000000000,
				// disp-lo mask - 6
				0b000000000000000000000000000000000000000000000000,
				// disp-hi mask - 7
				0b000000000000000000000000000000000000000000000000,
				// data1 mask - 8
				0b000000001111111100000000000000000000000000000000,
				// data2 mask - 9
				0b000000000000000011111111000000000000000000000000,
				// SR mask - 10
				0b000000000000000000000000000000000000000000000000,
				// addr-lo mask - 11
				0b000000000000000000000000000000000000000000000000,
				// addr-hi mask - 12
				0b000000000000000000000000000000000000000000000000,
			}}};

	while (1)
	{
		byte1 = loadInstructionByte(file, buffer, &i);
		if (byte1 == EOF)
		{
			break;
		}

		opcode = ((unsigned char)byte1);


		// update masks depending on opcode retrieved
		if ((opcode >> 2) == instructions[0][0][0])
		{

			// register / memory to / from register
			opcodeMask = instructions[0][1][0];
			dMask = instructions[0][1][1];
			wMask = instructions[0][1][2];
			modMask = instructions[0][1][3];
			regMask = instructions[0][1][4];
			rmMask = instructions[0][1][5];

			byte2 = loadInstructionByte(file, buffer, &i);

			uint64_t instruction = ((uint64_t)(uint8_t)byte1 << 40) |
								   ((uint64_t)(uint8_t)byte2 << 32);

			mod = (instruction & modMask) >> 38;

			if (mod == 0b01 || mod == 0b10)
			{
				byte3 = loadInstructionByte(file, buffer, &i);
				byte4 = loadInstructionByte(file, buffer, &i);
				uint64_t instruction = ((uint64_t)(uint8_t)byte1 << 40) |
									   ((uint64_t)(uint8_t)byte2 << 32) |
									   ((uint64_t)(uint8_t)byte3 << 24) |
									   ((uint64_t)(uint8_t)byte4 << 16);
			}

			printf("instruction: ");
			printf("%s ", byte_to_binary(byte1));
			printf("%s ", byte_to_binary(byte2));
			printf("%s ", byte_to_binary(byte3));
			printf("%s ", byte_to_binary(byte4));
			printf("%s ", byte_to_binary(0));
			printf("%s ", byte_to_binary(0));
			printf("\n");

			opcode = (instruction & opcodeMask) >> 42;

			d = (instruction & dMask) >> 41;
			w = (instruction & wMask) >> 40;

			reg = (instruction & regMask) >> 35;
			rm = (instruction & rmMask) >> 32;

			printf("register / memory to / from register\n");

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));

			printf("\n");
			printf("decoded: \n");

			// Direction
			if (d)
				printf("d: from\n");
			else
				printf("d: to\n");

			// Width
			if (w)
			{
				printf("w: word\n");
				regs[0] = "ax";
				regs[1] = "cx";
				regs[2] = "dx";
				regs[3] = "bx";
				regs[4] = "sp";
				regs[5] = "bp";
				regs[6] = "si";
				regs[7] = "di";
			}
			else
			{
				printf("w: byte\n");
				regs[0] = "al";
				regs[1] = "cl";
				regs[2] = "dl";
				regs[3] = "bl";
				regs[4] = "ah";
				regs[5] = "ch";
				regs[6] = "dh";
				regs[7] = "bh";
			}

			// addressing mode
			printf("mod: %s\n", byte_to_binary(mod));

			// register
			printf("reg: %s\n", regs[reg]);

			// register or memory operand
			if (1)
				printf("rm: %s\n", regs[rm]);

			printf("full instruction: \n");
			printf("%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);
			fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);

			printf("================\n\n");
		}
		else if ((opcode >> 4) == instructions[1][0][0])
		{
			// immidiate to register
			byte2 = loadInstructionByte(file, buffer, &i);
			byte3 = loadInstructionByte(file, buffer, &i);

			// register / memory to / from register
			opcodeMask = instructions[1][1][0];
			dMask = instructions[1][1][1];
			wMask = instructions[1][1][2];
			modMask = instructions[1][1][3];
			regMask = instructions[1][1][4];
			rmMask = instructions[1][1][5];
			data1Mask = instructions[1][1][8];
			data2Mask = instructions[1][1][9];

			uint64_t instruction = ((uint64_t)(uint8_t)byte1 << 40) |
								   ((uint64_t)(uint8_t)byte2 << 32) |
								   (uint64_t)(uint8_t)byte3 << 24;

			printf("instruction: ");
			printf("%s ", byte_to_binary(byte1));
			printf("%s ", byte_to_binary(byte2));
			printf("%s ", byte_to_binary(byte3));
			printf("\n");

			opcode = (instruction & opcodeMask) >> 44;
			d = 0;
			w = (instruction & wMask) >> 43;
			mod = 0;
			reg = (instruction & regMask) >> 40;
			rm = 0;
			data1 = (instruction & data1Mask) >> 32;
			data2 = (instruction & data2Mask) >> 24;

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));
			printf("data1: %s\n", byte_to_binary(data1));
			printf("data2: %s\n", byte_to_binary(data2));

			printf("\n");
			printf("decoded: \n");

			// Direction
			if (d)
				printf("d: from\n");
			else
				printf("d: to\n");

			// Width
			if (w)
			{
				printf("w: word\n");
				regs[0] = "ax";
				regs[1] = "cx";
				regs[2] = "dx";
				regs[3] = "bx";
				regs[4] = "sp";
				regs[5] = "bp";
				regs[6] = "si";
				regs[7] = "di";
			}
			else
			{
				printf("w: byte\n");
				regs[0] = "al";
				regs[1] = "cl";
				regs[2] = "dl";
				regs[3] = "bl";
				regs[4] = "ah";
				regs[5] = "ch";
				regs[6] = "dh";
				regs[7] = "bh";
			}

			// addressing mode
			printf("mod: %s\n", byte_to_binary(mod));

			// register
			printf("reg: %s\n", regs[reg]);

			// register or memory operand
			if (1)
				printf("rm: %s\n", regs[rm]);

			printf("full instruction: \n");
			printf("%s %s, %d\n", opcodes[opcode], regs[reg], data1);
			fprintf(fileOut, "%s %s, %d\n", opcodes[opcode], regs[reg], data1);

			printf("================\n\n");
		}
	}

	fclose(file);
	fclose(fileOut);

	printf("End of sim\n");

	return 0;
}
