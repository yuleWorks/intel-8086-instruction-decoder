#include "InstructionFunctions.h"

static const char* registerTableMod11[16] =
{
	"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
	"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
};

static const char* registerTableModDisp[16] =
{
	"[bx + si]",    "[bx + di]",	"[bp + si]",	"[bp + di]",	"[si]",		"[di]",		"direct addr",	"[bx]", 
	"bx + si",		"bx + di",		"bp + si",		"bp + di",		"si",		"di",		"bp",			"bx"	// d8 or d16
};

static const char* byteWordTable[3] =
{
	" byte ", " word ", " "
};

static const char* addMovSubCmpTable[8] =
{
	"add", "mov", "adc", "sbb", "EMPTY", "sub", "EMPTY", "cmp" // adc and sbb are not yet used
//   000    001    010    011     100     101     110     111
};

static const char* jumpTable[16] =
{
	"jo", "jno", "jb", "jnb", "je", "jnz", "jbe", "ja",
	"js", "jns", "jp", "jnp", "jl", "jnl", "jle", "jg"
};

static const char* loopTable[4] =
{
	"loopnz", "loopz", "loop", "jcxz"
};

static const char* setByteTable[7] =
{
	"inc", "dec", "call", "call", "jmp", "jmp", "push"
};

void getRegNameAndEAC(char* out, size_t outSize, uint8_t rm, uint8_t w, uint8_t mod, int16_t displacement) // uint8 rm, bool w, uint8 mod, int16 disp
{
	if (mod == 0b00000011)
	{
		snprintf(out, outSize, "%s", registerTableMod11[(w << 3) | (rm & 0b00000111)]);
	}
	else if (mod == 0b00000000)
	{
		if (rm == 0b00000110)
		{
			// direct address
			// return "[%s]" where %s is 16 bit displacement from DISP-LO and DISP-HI
			snprintf(out, outSize, "[%d]", displacement);
		}
		else
		{
			snprintf(out, outSize, "%s", registerTableModDisp[(rm & 0b00000111)]);
		}
	}
	else if (mod == 0b00000001 || mod == 0b00000010)
	{ 
		int16_t maskedDisplacement = displacement & (0b1111111111111111 >> (((mod & 0b00000001) && (displacement > 0)) * 8)); // when mod = 01, shift, when mod = 10, do not shift. Shift+mask grabs 1 or 2 bytes for displacement.
		if (maskedDisplacement == 0)
		{
			snprintf(out, outSize, "[%s]", registerTableModDisp[0b00001000 | (rm & 0b00000111)]);
		}else
		{
			char sign = maskedDisplacement < 0 ? '-' : '+';
			snprintf(out, outSize, "[%s %c %d]", registerTableModDisp[0b00001000 | (rm & 0b00000111)], sign, abs(maskedDisplacement));
		}
	}
	else {
		snprintf(out, outSize, "ERROR");
	}
}

uint16_t f_print_mov_add_sub_cmp_RegOrMemToFromReg_100010(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2-4
	//__XXX_10 11000111 00000000 11111111 00000000 00000000 00000000 00000000
	//   |  DW | REG |  DISPLO   DISPHI   XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
	// name   MOD   R/M
	//d = 0 R/M = destination REG = source       
	//d = 1 REG = destination R/M = source
	uint8_t mod;	
	if (remainingBytes >= 2) // 2 is minimum instruction size
	{
		mod = ((uint8_t)buffer[1] & 0b11000000)>>6;
	}
	else
	{
		printf("instructions are not properly aligned with file size.");
		exit(1); // error, instructions are offset with ending of file
	}

	uint8_t requiredBytes = 2;
	uint8_t rm = ((uint8_t)buffer[1] & 0b00000111);
	

	switch (mod)
	{
	case(0b00000010): // 16 bit displacement was used
		requiredBytes = 4;
		break;
	case(0b00000001): // 8 bit displacement was used
		requiredBytes = 3;
		break;
	case(0b00000000):
		if (rm == 0b00000110) // direct address (16 bit) was used
		{
			requiredBytes = 4;
			break;
		}
		break;
	}
	
	if (requiredBytes > remainingBytes)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1); 
	}

	int16_t disp = 0;
	switch (requiredBytes)
	{
	case(4):
		disp = (uint8_t)buffer[2] | ((int16_t)buffer[3] << 8);
		break;
	case(3):
		disp = (int8_t)buffer[2];
		break;
	}

	uint8_t d =		((uint8_t)buffer[0] & 0b00000010) >> 1;
	uint8_t w =		(uint8_t)buffer[0] & 0b00000001;	
	uint8_t reg =	((uint8_t)buffer[1] & 0b00111000) >> 3;

	char destinationStr[64];
	char sourceStr[64];

	if (d == 0)
	{
		getRegNameAndEAC(destinationStr, sizeof(destinationStr), rm, w, mod, disp);
		snprintf(sourceStr, sizeof(sourceStr), "%s", registerTableMod11[(w << 3) | (reg & 0b00000111)]);
	}
	else
	{
		snprintf(destinationStr, sizeof(destinationStr), "%s", registerTableMod11[(w << 3) | (reg & 0b00000111)]);
		getRegNameAndEAC(sourceStr, sizeof(sourceStr), rm, w, mod, disp);
	}

	const char* mnemonicStr = addMovSubCmpTable[(buffer[0] & 0b00111000) >> 3];
	printf("%s %s, %s\n", mnemonicStr, destinationStr, sourceStr);

	//size of instruction in bytes
	return requiredBytes;
	
}

uint16_t f_print_mov_add_sub_cmp_ImmToRegOrMem_1100011(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 3-6
	//_______0 11___000 11111111 00000000 11111111 00000000
	//       W |name |  DISPLO   DISPHI   DATALO   DATAHI
	//		  MOD   RM  

	uint8_t mod;
	if (remainingBytes >= 3) //3 is minimum instruction size
	{
		mod = ((uint8_t)buffer[1] & 0b11000000) >> 6;
	}
	else
	{
		printf("instructions are not properly aligned with file size.");
		exit(1); //error, instructions are offset with ending of file
	}

	uint8_t rm = (uint8_t)buffer[1] & 0b00000111;
	uint8_t w = (uint8_t)buffer[0] & 0b00000001;
	uint8_t requiredBytes = 3;
	uint8_t dispBytes = 0;

	switch (mod)
	{
	case(0b00000010): // 16 bit displacement was used
		dispBytes = 2;
		break;
	case(0b00000001): // 8 bit displacement was used
		dispBytes = 1;
		break;
	case(0b00000000):
		if (rm == 0b00000110) // direct address (16 bit) was used
		{
			dispBytes =  2;
			break;
		}
		break;
	}
	uint8_t s = 0;
	if ((buffer[0] & 0b11111110) != 0b11000110) s = (buffer[0] & 0b00000010) >> 1; // need to grab sign bit for ADD/SUB/CMP data due to operation being done. 

	requiredBytes = requiredBytes + dispBytes + (w && !s);

	if (requiredBytes > remainingBytes)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1);
	}
	
	uint16_t disp = 0;
	uint16_t data = 0;
	
	switch (dispBytes)
	{
	case(2):
		disp = (uint8_t)buffer[2] | ((int16_t)buffer[3] << 8);
		break;
	case(1):
		disp = (int8_t)buffer[2];
		break;
	}

	const char* mnemonicStr;

	const char* leadingSizeMarker;
	const char* endingSizeMarker;

	if ((buffer[0] & 0b11111110) == 0b11000110) // check if instruction is mov since ADD has the same 000 bits in buffer[1].
	{
		mnemonicStr = addMovSubCmpTable[1];
		leadingSizeMarker = byteWordTable[2];
		if (mod != 0b00000011) endingSizeMarker = byteWordTable[w];
		else endingSizeMarker = byteWordTable[2];
	}
	else
	{
		mnemonicStr = addMovSubCmpTable[(buffer[1] & 0b00111000) >> 3];
		if (mod != 0b00000011) leadingSizeMarker = byteWordTable[w];
		else leadingSizeMarker = byteWordTable[2];
		endingSizeMarker = byteWordTable[2];
	}

	if (w)
	{
		if (s == 0) data = (uint16_t)buffer[2 + dispBytes] | ((uint16_t)buffer[2 + dispBytes + 1] << 8);
		else		data = ((int16_t)buffer[2 + dispBytes]);		
	}
	else
	{
		data = (uint16_t)buffer[2 + dispBytes];
	}
	
	char destinationStr[64];
	char sourceStr[64];

	getRegNameAndEAC(destinationStr, sizeof(destinationStr), rm, w, mod, disp);
	printf("%s%s%s,%s%d\n", mnemonicStr, leadingSizeMarker, destinationStr, endingSizeMarker, data);

	return requiredBytes;
}

uint16_t f_print_mov_ImmToReg_1011(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2-3
	//00001000 11111111 00000000
	//____w    data     data w=1
	//     REG
	uint8_t w = ((uint8_t)buffer[0] & 0b00001000) >> 3;

	uint8_t requiredBytes = 2+w;	
	
	if (requiredBytes > remainingBytes)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1);
	}

	int16_t data = (int8_t)buffer[1]; //data represents signed 8bit value
	

	if(w == 1) data = ((int16_t)buffer[2] << 8) | (uint8_t)buffer[1]; // top of data represents signed 8bit value, 
																	  // bottom is unsigned (only hi bits matter for 2s compliment)

	uint8_t reg = ((uint8_t)buffer[0] & 0b00000111);

 	const char* destinationStr = (const char*)registerTableMod11[(w << 3) | (reg & 0b00000111)];
	
	printf("mov %s, %d\n", destinationStr, data);

	return requiredBytes;
}

uint16_t f_print_mov_MemToAcc_1010000(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2-3
	//_______0 11111111 00000000 
	//       W ADDRLO	ADDRHI
	uint8_t w = (uint8_t)buffer[0] & 0b00000001;
	uint8_t requiredBytes = 2 + w;
	if (requiredBytes > remainingBytes)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1);
	}
	uint16_t address = 0;
	if (w)
	{
		address = buffer[1] | buffer[2] << 8;
	}
	else
	{
		address = buffer[1];
	}

	printf("mov ax, [%d]\n", address);
	return requiredBytes;
}

uint16_t f_print_mov_AccToMem_1010001(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2-3
	//_______0 11111111 00000000 
	//       W ADDRLO	ADDRHI
	uint8_t w = (uint8_t)buffer[0] & 0b00000001;
	uint8_t requiredBytes = 2 + w;
	if (requiredBytes > remainingBytes)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1);
	}
	uint16_t address = 0;
	if (w)
	{
		address = buffer[1] | buffer[2] << 8;
	}
	else
	{
		address = buffer[1];
	}

	printf("mov [%d], ax\n", address);
	return requiredBytes;
}

uint16_t f_print_add_sub_cmp_ImmToAcc(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2-3
	//__XXX__1 00000000  11111111
	//  inst W data      w=1 data	 
	uint8_t w = (uint8_t)buffer[0] & 0b00000001;
	uint8_t requiredBytes = 2 + w;
	if (requiredBytes > remainingBytes)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1);
	}
	int16_t data = 0;
	if (w)
	{
		data = (uint8_t)buffer[1] | (int16_t)buffer[2] << 8;
	}
	else
	{
		data = (int8_t)buffer[1];
	}

	const char* accumStr = registerTableMod11[0b00000000 | (w << 3)];
	const char* mnemonicStr = addMovSubCmpTable[(buffer[0] & 0b00111000)>>3];

	printf("%s %s, %d\n", mnemonicStr, accumStr, data);
	return requiredBytes;
}

uint16_t f_print_jumps_many(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2
	//____1111 00000000
	//    type jlocation
	/*    
	jo        01110000
	jno       01110001
	jb        01110010
	jnb       01110011
	je        01110100
	jnz, jne  01110101
	jbe       01110110
	ja        01110111
	js        01111000
	jns       01111001
	jp        01111010
	jnp       01111011
	jl        01111100
	jnl       01111101
	jle       01111110
	jg        01111111
	loopnz    11100000
	loopz     11100001
	loop      11100010
	jcxz      11100011	
	*/
	if (remainingBytes < 2)
	{
		printf("instruction required more bytes than are left in the buffer.");
		exit(1);
	}

	const char* jumpMnemonic = jumpTable[buffer[0] & 0b00001111];
	int8_t offset = buffer[1] + 2;
	char sign = '+';
	if (offset < 0) sign = '-';
	printf("%s $%c%d\n", jumpMnemonic, sign, abs(offset));
	return 2;
}

uint16_t f_print_loops_many(unsigned char* buffer, long remainingBytes)
{
	//bytes needed: 2
	//______11 00000000
	//    type jlocation
	const char* loopMnemonic = loopTable[buffer[0] & 0b00000011];
	int8_t offset = buffer[1] + 2;
	char sign = '+';
	if (offset < 0) sign = '-';
	printf("%s $%c%d\n", loopMnemonic, sign, abs(offset));
	return 2;
}


static uint16_t f_print_completionist_inc_dec_call_jmp_push_1111111(unsigned char* buffer, long remainingbytes)
{
	//
	//INC: 1111111W XX000XXX XXXXXXXX XXXXXXXX
	//				MOD   RM  DISPLO   DISPHI 
	//DEC: 1111111W XX001XXX  XXXXXXXX XXXXXXXX
	//				MOD   RM  DISPLO   DISPHI 

	// 
	//CALL INDIRECT WITHIN SEGMENT (requires 2 other instructions also needed to be decoded elsewhere for call)
	//	   11111111 XX010XXX XXXXXXXX XXXXXXXX
	//              MOD   RM  DISPLO   DISPHI
	//
	//CALL INDIRECT INTERSEGMENT 
	//	   11111111 XX011XXX XXXXXXXX XXXXXXXX
	//              MOD   RM  DISPLO   DISPHI
	//
	//JMP INDIRECT WITHIN SEGMENT (requires 2 other instructions also needed to be decoded elsewhere for jmp)
	//	   11111111 XX100XXX XXXXXXXX XXXXXXXX
	//              MOD   RM  DISPLO   DISPHI
	//
	//JMP INDIRECT INTERSEGMENT
	//	   11111111 XX101XXX XXXXXXXX XXXXXXXX
	//              MOD   RM  DISPLO   DISPHI
	// 
	//PUSH:11111111 XX110XXX XXXXXXXX XXXXXXXX (requires 2 other instructions also needed to be decoded elsewhere for push)
	// 				MOD   RM  DISPLO   DISPHI 

	return 0;
}	

