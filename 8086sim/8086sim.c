#include <stdio.h>
#include <string.h> /* strcat */
#include <stdlib.h> /* strtol */

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
	char byte1, byte2;
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
				  data = 0;

	unsigned int opcodeMask = 0b11111111,
				  dMask = 0,
				  wMask = 0,
				  modMask = 0,
				  regMask = 0,
				  rmMask = 0,
				  dataMask = 0;

	fprintf(fileOut, "bits 16\n");

	while (1)
	{
		byte1 = getc(file);
		byte2 = getc(file);
		if (byte1 == EOF || byte2 == EOF)
		{
			break;
		}

		printf("bytes: ");
		printf(" %s ", byte_to_binary(byte1));
		printf(" %s ", byte_to_binary(byte2));
		buffer[i++] = byte1;
		buffer[i++] = byte2;
		printf("\n\n");

		opcode = ((unsigned char)byte1 & opcodeMask);

		// update masks depending on opcode retrieved
		if ((opcode >> 2) == 0b0100010)
		{
			// register / memory to / from register
			opcodeMask = 0b11111100;
			dMask = 0b00000010;
			wMask = 0b00000001;
			modMask = 0b11000000;
			regMask = 0b00111000;
			rmMask = 0b00000111;

			opcode = ((unsigned char)byte1 & opcodeMask) >> 2;

			d = ((unsigned char)byte1 & dMask) >> 1;
			w = ((unsigned char)byte1 & wMask);

			mod = ((unsigned char)byte2 & modMask) >> 6;
			reg = ((unsigned char)byte2 & regMask) >> 3;
			rm = ((unsigned char)byte2 & rmMask);

			printf("register / memory to / from register\n");

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));

			printf("\n");
			printf("decoded: \n");
			printf("opcode: %s\n", opcodes[opcode]);

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
		else if ((opcode >> 4) == 0b1011)
		{
			// immidiate to register
			opcodeMask = 0b1111000000000000;
			dMask = 0b0000000000000000;
			wMask = 0b0000100000000000;
			modMask = 0b0000000000000000;
			regMask = 0b0000011100000000;
			rmMask = 0b0000000000000000;
			dataMask = 0b0000000011111111;

			int instruction = (byte1 << 8) | byte2;

			opcode = (instruction & opcodeMask) >> 12;

			d = (instruction & dMask);
			w = (instruction & wMask) >> 11;

			mod = (instruction & modMask);
			reg = (instruction & regMask) >> 8;
			rm = (instruction & rmMask);
			data = (instruction & dataMask);

			printf("immidiate to register/memory opcode: %s\n", byte_to_binary(opcode));

			printf("opcode: %s\n", byte_to_binary(opcode));
			printf("d: %s\n", byte_to_binary(d));
			printf("w: %s\n", byte_to_binary(w));
			printf("mod: %s\n", byte_to_binary(mod));
			printf("reg: %s\n", byte_to_binary(reg));
			printf("rm: %s\n", byte_to_binary(rm));

			printf("\n");
			printf("decoded: \n");
			printf("opcode: %s\n", opcodes[opcode]);

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
			printf("%s %s, %d\n", opcodes[opcode], regs[reg], data);
			fprintf(fileOut, "%s %s, %d\n", opcodes[opcode], regs[reg], data);

			printf("================\n\n");
		}
	}

	fclose(file);
	fclose(fileOut);

	printf("End of sim\n");

	return 0;
}
