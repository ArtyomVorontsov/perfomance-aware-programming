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
 * Support different instructions, such as immidiate, add, sub and cmp
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
	char byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0, byte5 = 0, byte6 = 0;
	int i = 0;

	char *opcodes[64],
		ds[2],
		ws[2],
		mods[4],
		*regs[8],
		rms[8];

	opcodes[34] = "mov";
	opcodes[11] = "mov";
	opcodes[32] = "add";

	unsigned int opcode = 0,
				 d = 0,
				 w = 0,
				 s = 0,
				 mod = 0,
				 reg = 0,
				 rm = 0,
				 data1 = 0,
				 data2 = 0,
				 dispLo = 0,
				 dispHi = 0;

	uint64_t data = 0;

	uint64_t opcodeMask = 0b11111111,
			 dMask = 0,
			 wMask = 0,
			 modMask = 0,
			 regMask = 0,
			 rmMask = 0,
			 data1Mask = 0,
			 data2Mask = 0,
			 dispLoMask = 0,
			 dispHiMask = 0,
			 sMask = 0;

	fprintf(fileOut, "bits 16\n");

	uint64_t instructions[5][2][14] = {
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
			}},

		// add
		{
			// opcode
			{0b000000},
			{
				// opcode mask - 0
				0b111111000000000000000000000000000000000000000000,
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
			}},

		// add immidiate
		{
			// opcode
			{0b100000},
			{
				// opcode mask - 0
				0b111111000000000000000000000000000000000000000000,
				// d mask - 1
				0b000000100000000000000000000000000000000000000000,
				// w mask - 2
				0b000000010000000000000000000000000000000000000000,
				// mod mask - 3
				0b000000001100000000000000000000000000000000000000,
				// reg mask - 4
				0b000000000000000000000000000000000000000000000000,
				// r/m mask - 5
				0b000000000000011100000000000000000000000000000000,
				// disp-lo mask - 6
				0b000000000000000011111111000000000000000000000000,
				// disp-hi mask - 7
				0b000000000000000000000000111111110000000000000000,
				// data1 mask - 8
				0b000000000000000000000000000000001111111100000000,
				// data2 mask - 9
				0b000000000000000000000000000000000000000011111111,
				// data1 mask (when mod == 0b11) - 10
				0b000000000000000011111111000000000000000000000000,				
				// data2 mask (when mod == 0b11) - 11
				0b000000000000000000000000111111110000000000000000,
				// SR mask - 12
				0b000000000000000000000000000000000000000000000000,
				// addr-lo mask - 13
				0b000000000000000000000000000000000000000000000000,
				// addr-hi mask - 14
				0b000000000000000000000000000000000000000000000000,
				// s mask - 15
				0b000000100000000000000000000000000000000000000000,
			}},

		// sub
		{
			// opcode
			{0b001010},
			{
				// opcode mask - 0
				0b111111000000000000000000000000000000000000000000,
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
			}},

		// cmp
		{
			// opcode
			{0b001110},
			{
				// opcode mask - 0
				0b111111000000000000000000000000000000000000000000,
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
			}}};

	while (1)
	{
		byte1 = loadInstructionByte(file, buffer, &i);
		if (byte1 == EOF)
		{
			printf("EOF\n");
			break;
		}

		opcode = ((unsigned char)byte1);

		printf("opcode %d\n", opcode);

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
			dispLoMask = instructions[0][1][6];
			dispHiMask = instructions[0][1][7];

			byte2 = loadInstructionByte(file, buffer, &i);

			uint64_t instruction = ((uint64_t)(uint8_t)byte1 << 40) |
								   ((uint64_t)(uint8_t)byte2 << 32);

			mod = (instruction & modMask) >> 38;

			if (mod == 0b01 || mod == 0b10)
			{
				// 8 bit dispacement
				if (mod == 0b01)
				{
					byte3 = loadInstructionByte(file, buffer, &i);
				}

				// 16 bit dispacement
				if (mod == 0b10)
				{
					byte3 = loadInstructionByte(file, buffer, &i);
					byte4 = loadInstructionByte(file, buffer, &i);
				}

				instruction = ((uint64_t)(uint8_t)byte1 << 40) |
							  ((uint64_t)(uint8_t)byte2 << 32) |
							  ((uint64_t)(uint8_t)byte3 << 24) |
							  ((uint64_t)(uint8_t)byte4 << 16);
			}

			printf("instruction: ");
			printf("%s ", byte_to_binary(instruction >> 40));
			printf("%s ", byte_to_binary(instruction >> 32));
			printf("%s ", byte_to_binary(instruction >> 24));
			printf("%s ", byte_to_binary(instruction >> 16));
			printf("%s ", byte_to_binary(0));
			printf("%s ", byte_to_binary(0));
			printf("\n");

			opcode = (instruction & opcodeMask) >> 42;

			d = (instruction & dMask) >> 41;
			w = (instruction & wMask) >> 40;

			reg = (instruction & regMask) >> 35;
			rm = (instruction & rmMask) >> 32;
			dispLo = (instruction & dispLoMask) >> 24;
			dispHi = (instruction & dispHiMask) >> 16;

			printf("register / memory to / from register\n");

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));
			printf("dispLo: %s\n", byte_to_binary(dispLo));
			printf("dispHi: %s\n", byte_to_binary(dispHi));

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
			if (mod != 0b11)
			{
				char rmOp[100];
				rmOp[0] = '\0';
				char dispNumber[16];

				switch (rm)
				{

				case 0b000:
					strcat(rmOp, "[bx + si");
					break;
				case 0b001:
					strcat(rmOp, "[bx + di");
					break;
				case 0b010:
					strcat(rmOp, "[bp + si");
					break;
				case 0b011:
					strcat(rmOp, "[bp + di");
					break;
				case 0b100:
					strcat(rmOp, "[si");
					break;
				case 0b101:
					strcat(rmOp, "[di");
					break;
				case 0b110:
					strcat(rmOp, "[bp");
					break;
				case 0b111:
					strcat(rmOp, "[bx");
					break;

				default:
					break;
				}

				if (mod == 0b00)
				{
					strcat(rmOp, "]");
				}
				else
				{

					if (mod == 0b01)
					{
						sprintf(dispNumber, "%d", (signed)(int)dispLo);
					}
					else if (mod == 0b10)
					{
						sprintf(dispNumber, "%d", (signed)(int)((dispHi << 8) | dispLo));
					}

					if (dispNumber[0] == '0')
					{
						strcat(rmOp, "]");
					}
					else
					{
						strcat(strcat(strcat(rmOp, " + "), dispNumber), "]");
					}
				}

				printf("full instruction: \n");

				if (d == 1)
				{
					printf("%s %s, %s\n", opcodes[opcode], regs[reg], rmOp);
					fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], regs[reg], rmOp);
				}
				else
				{
					printf("%s %s, %s\n", opcodes[opcode], rmOp, regs[reg]);
					fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], rmOp, regs[reg]);
				}
			}
			else
			{

				printf("full instruction: \n");
				printf("%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);
				fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);
			}
			printf("================\n\n");
		}
		else if ((opcode >> 4) == instructions[1][0][0])
		{
			// immidiate to register
			byte2 = loadInstructionByte(file, buffer, &i);

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
								   ((uint64_t)(uint8_t)byte2 << 32);

			w = (instruction & wMask) >> 43;

			if (w)
			{
				byte3 = loadInstructionByte(file, buffer, &i);
				instruction = ((uint64_t)(uint8_t)byte1 << 40) |
							  ((uint64_t)(uint8_t)byte2 << 32) |
							  (uint64_t)(uint8_t)byte3 << 24;
			}

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
			printf("data: %s\n", byte_to_binary(data));
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

				data = (data2 << 8) | data1;
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

				data = data1;
			}

			// addressing mode
			printf("mod: %s\n", byte_to_binary(mod));

			// register
			printf("reg: %s\n", regs[reg]);

			// register or memory operand
			if (1)
				printf("rm: %s\n", regs[rm]);

			printf("full instruction: \n");
			printf("%s %s, %d\n", opcodes[opcode], regs[reg], data);
			fprintf(fileOut, "%s %s, %d\n", opcodes[opcode], regs[reg], data);

			printf("================\n\n");
		}
		else if ((opcode >> 2) == instructions[2][0][0])
		{
			// register / memory to / from register
			opcodeMask = instructions[2][1][0];
			dMask = instructions[2][1][1];
			wMask = instructions[2][1][2];
			modMask = instructions[2][1][3];
			regMask = instructions[2][1][4];
			rmMask = instructions[2][1][5];
			dispLoMask = instructions[2][1][6];
			dispHiMask = instructions[2][1][7];

			byte2 = loadInstructionByte(file, buffer, &i);

			uint64_t instruction = ((uint64_t)(uint8_t)byte1 << 40) |
								   ((uint64_t)(uint8_t)byte2 << 32);

			mod = (instruction & modMask) >> 38;

			if (mod == 0b01 || mod == 0b10)
			{
				// 8 bit dispacement
				if (mod == 0b01)
				{
					byte3 = loadInstructionByte(file, buffer, &i);
				}

				// 16 bit dispacement
				if (mod == 0b10)
				{
					byte3 = loadInstructionByte(file, buffer, &i);
					byte4 = loadInstructionByte(file, buffer, &i);
				}

				instruction = ((uint64_t)(uint8_t)byte1 << 40) |
							  ((uint64_t)(uint8_t)byte2 << 32) |
							  ((uint64_t)(uint8_t)byte3 << 24) |
							  ((uint64_t)(uint8_t)byte4 << 16);
			}

			printf("instruction: ");
			printf("%s ", byte_to_binary(instruction >> 40));
			printf("%s ", byte_to_binary(instruction >> 32));
			printf("%s ", byte_to_binary(instruction >> 24));
			printf("%s ", byte_to_binary(instruction >> 16));
			printf("%s ", byte_to_binary(0));
			printf("%s ", byte_to_binary(0));
			printf("\n");

			opcode = (instruction & opcodeMask) >> 42;

			d = (instruction & dMask) >> 41;
			w = (instruction & wMask) >> 40;

			reg = (instruction & regMask) >> 35;
			rm = (instruction & rmMask) >> 32;
			dispLo = (instruction & dispLoMask) >> 24;
			dispHi = (instruction & dispHiMask) >> 16;

			printf("add\n");

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));
			printf("dispLo: %s\n", byte_to_binary(dispLo));
			printf("dispHi: %s\n", byte_to_binary(dispHi));

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
			if (mod != 0b11)
			{
				char rmOp[100];
				rmOp[0] = '\0';
				char dispNumber[16];

				switch (rm)
				{

				case 0b000:
					strcat(rmOp, "[bx + si");
					break;
				case 0b001:
					strcat(rmOp, "[bx + di");
					break;
				case 0b010:
					strcat(rmOp, "[bp + si");
					break;
				case 0b011:
					strcat(rmOp, "[bp + di");
					break;
				case 0b100:
					strcat(rmOp, "[si");
					break;
				case 0b101:
					strcat(rmOp, "[di");
					break;
				case 0b110:
					strcat(rmOp, "[bp");
					break;
				case 0b111:
					strcat(rmOp, "[bx");
					break;

				default:
					break;
				}

				if (mod == 0b00)
				{
					strcat(rmOp, "]");
				}
				else
				{

					if (mod == 0b01)
					{
						sprintf(dispNumber, "%d", (signed)(int)dispLo);
					}
					else if (mod == 0b10)
					{
						sprintf(dispNumber, "%d", (signed)(int)((dispHi << 8) | dispLo));
					}

					if (dispNumber[0] == '0')
					{
						strcat(rmOp, "]");
					}
					else
					{
						strcat(strcat(strcat(rmOp, " + "), dispNumber), "]");
					}
				}

				printf("full instruction: 1\n");

				if (d == 1)
				{
					printf("%d\n", opcode);
					printf("%s %s, %s\n", opcodes[opcode], regs[reg], rmOp);
					fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], regs[reg], rmOp);
				}
				else
				{
					printf("%s %s, %s\n", opcodes[opcode], rmOp, regs[reg]);
					fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], rmOp, regs[reg]);
				}
			}
			else
			{

				printf("full instruction: \n");
				printf("%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);
				fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);
			}
			printf("================\n\n");
		}
		else if ((opcode >> 2) == instructions[3][0][0])
		{
			// immidiate to register
			byte2 = loadInstructionByte(file, buffer, &i);

			// register / memory to / from register
			opcodeMask = instructions[3][1][0];
			dMask = instructions[3][1][1];
			wMask = instructions[3][1][2];
			modMask = instructions[3][1][3];
			regMask = instructions[3][1][4];
			rmMask = instructions[3][1][5];
			data1Mask = instructions[3][1][8];
			data2Mask = instructions[3][1][9];
			sMask = instructions[3][1][15];

			uint64_t instruction = ((uint64_t)(uint8_t)byte1 << 40) |
								   ((uint64_t)(uint8_t)byte2 << 32);

			w = (instruction & wMask) >> 40;

			if (w)
			{
				byte3 = loadInstructionByte(file, buffer, &i);
				instruction = ((uint64_t)(uint8_t)byte1 << 40) |
							  ((uint64_t)(uint8_t)byte2 << 32) |
							  (uint64_t)(uint8_t)byte3 << 24;
			}

			printf("instruction: ");
			printf("%s ", byte_to_binary(byte1));
			printf("%s ", byte_to_binary(byte2));
			printf("%s ", byte_to_binary(byte3));
			printf("\n");

			opcode = (instruction & opcodeMask) >> 42;
			mod = (instruction & modMask) >> 38;
			d = 0;
			s = (instruction & sMask) >> 41;
			w = (instruction & wMask) >> 40;
			reg = 0;
			rm = (instruction & rmMask) >> 32;


			if (mod == 0b11)
			{
				data1Mask = instructions[3][1][10];
				data2Mask = instructions[3][1][11];
			}

			if (mod == 0b11)
			{
				data1 = (instruction & data1Mask) >> 24;
				data2 = (instruction & data2Mask) >> 16;
			}

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("s: %s\n", byte_to_binary(s));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));
			printf("data: %s\n", byte_to_binary(data));
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

				data = (data2 << 8) | data1;
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

				data = data1;
			}

			// addressing mode
			printf("mod: %s\n", byte_to_binary(mod));

			// register
			printf("reg: %s\n", regs[reg]);

			// register or memory operand
			if (1)
				printf("rm: %s\n", regs[rm]);

			printf("full instruction: \n");
			printf("opcodes %d\n", opcode);

			printf("%s %s, %d\n", opcodes[opcode], regs[rm], data);
			fprintf(fileOut, "%s %s, %d\n", opcodes[opcode], regs[rm], data);

			printf("================\n\n");
		}
	}

	fclose(file);
	fclose(fileOut);

	printf("End of sim\n");

	return 0;
}
