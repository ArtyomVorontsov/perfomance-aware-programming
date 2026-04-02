#include <stdio.h>
#include <string.h>     /* strcat */
#include <stdlib.h>     /* strtol */

/* 8086 simulator */
/*
 * TODO:
 * Read binary file - done
 * Disassembly file
 *
 */

const char *byte_to_binary(int x)
{
	static char b[9];
	b[0] = '\0';

 	int z;
     	for (z = 128; z > 0; z >>= 1){
     		strcat(b, ((x & z) == z) ? "1" : "0");
     	}

        return b;
}

int main(int args){
	FILE *file = fopen("8086ext", "r");
	FILE *fileOut = fopen("8086ext.out", "w");

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

	unsigned char opcode = 0, 
	     d = 0, 
	     w = 0, 
	     mod = 0, 
	     reg = 0, 
	     rm = 0;

	unsigned char opcodeMask = 0b11111100, 
	     dMask = 0b00000010, 
	     wMask = 0b00000001, 
	     modMask = 0b11000000, 
	     regMask= 0b00111000, 
	     rmMask= 0b00000111;


	while(1){
		byte1 = getc(file);
		byte2 = getc(file);
		if(byte1 == EOF || byte2 == EOF){
			break;
		}

		printf("bytes: ");
		printf(" %s ", byte_to_binary(byte1));
		printf(" %s ", byte_to_binary(byte2));
		buffer[i++] = byte1;
		buffer[i++] = byte2;
		printf("\n\n");

		opcode = ((unsigned char) byte1 & opcodeMask) >> 2;
		d = ((unsigned char) byte1 & dMask) >> 1;
		w = ((unsigned char) byte1 & wMask);

		mod = ((unsigned char) byte2 & modMask) >> 6;
		reg = ((unsigned char) byte2 & regMask) >> 3;
		rm = ((unsigned char) byte2 & rmMask);


		printf("opcode: %s\n", byte_to_binary(opcode));
		printf("d: %s\n", byte_to_binary(d));
		printf("w: %s\n", byte_to_binary(w));
		printf("mod: %s\n", byte_to_binary(mod));
		printf("reg: %s\n", byte_to_binary(reg));
		printf("rm: %s\n", byte_to_binary(rm));


		printf("\n");
		printf("decoded: \n");
		printf("opcode: %s\n", opcodes[opcode]);

		if(d)
			printf("d: from\n");
		else
			printf("d: to\n");

		if(w){
			printf("w: word\n");
			regs[0] = "ax";
			regs[1] = "cx";
			regs[2] = "dx";
			regs[3] = "bx";
			regs[4] = "sp";
			regs[5] = "bp";
			regs[6] = "si";
			regs[7] = "di";
		} else {
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


		printf("mod: %s\n", byte_to_binary(mod));
		printf("reg: %s\n", regs[reg]);

		if(1)
			printf("rm: %s\n", regs[rm]);


		printf("full instruction: \n");
		printf("%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);
		fprintf(fileOut, "%s %s, %s\n", opcodes[opcode], regs[rm], regs[reg]);


		printf("================\n\n");

	}

	fclose(file);
	fclose(fileOut);

	printf("%s\n", buffer);
	printf("End of sim\n");

	return 0;
}

