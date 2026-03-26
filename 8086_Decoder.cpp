#include "InstructionFunctions.h"
#include <conio.h> // for _getch()

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

enum PrintMode
{
    IMMEDIATE,
    STEP,
    MINIMAL
};

typedef struct InstructionData {
    uint8_t instruction;        // Instruction bits, shifted to the right until no variable bits (like D, S, or W) are represented.
    uint8_t instructionBitSize; // Instruction bit size starting from lowest order bit (length to the left in bits)
    uint16_t(*f_print_instruction)(unsigned char*, long);
} InstructionData;

const struct InstructionData Instructions[] =
{
    /*MOV register/memory to/from register*/
    {0b00100010, 6, f_print_mov_add_sub_cmp_RegOrMemToFromReg_100010},

    /*MOV immediate to register/memory*/
    {0b01100011, 7, f_print_mov_add_sub_cmp_ImmToRegOrMem_1100011},

    /*MOV immediate to register*/
    {0b00001011, 4, f_print_mov_ImmToReg_1011},

    /*MOV memory to accumulator*/
    {0b01010000, 7, f_print_mov_MemToAcc_1010000},

    /*MOV accumulator to memory (SAME AS ABOVE, USE SAME INSTRUCTION DATA FOR memory <-> accumulator?*/
    {0b01010001, 7, f_print_mov_AccToMem_1010001},

    /*ADD SUB CMP immediate to register/memory*/
    {0b00100000, 6, f_print_mov_add_sub_cmp_ImmToRegOrMem_1100011},

    /*ADD register/memory with register to either*/
    {0b00000000, 6, f_print_mov_add_sub_cmp_RegOrMemToFromReg_100010},

    /*ADD immediate to accumulator*/
    {0b00000010, 7, f_print_add_sub_cmp_ImmToAcc},

    /*SUB register/memory with register to either*/
    {0b00001010, 6, f_print_mov_add_sub_cmp_RegOrMemToFromReg_100010},

    /*SUB immediate to accumulator*/
    {0b00010110, 7, f_print_add_sub_cmp_ImmToAcc},

    /*CMP register/memory with register to either*/
    {0b00001110, 6, f_print_mov_add_sub_cmp_RegOrMemToFromReg_100010},

    /*CMP immediate to accumulator*/
    {0b00011110, 7, f_print_add_sub_cmp_ImmToAcc},

    /*JO, JNO, JB, JNB, JE, JNE, JBE, JNBE, JS, JNS, JP, JNP, JL, JNL, JLE, JNLE*/
    {0b00000111, 4, f_print_jumps_many},

    /*LOOP, LOOPZ, LOOPNZ, JCXZ*/
    {0b00111000, 6, f_print_loops_many},

    /*INC DEC CALL JMP PUSH (includes 11111110 and 11111111 encodings... some treat last bit as instruction, others treat as W. Test is within function.)*/
};


void parseArgs(int argc, char** argv, uint8_t** out_flagArgvIndices, uint8_t** out_fileArgvIndices, uint8_t* out_flagCount, uint8_t* out_fileCount, enum PrintMode* printMode)
{
    uint8_t* flagArgvIndices;
    uint8_t* fileArgvIndices;
    uint8_t flagCount = 0;
    uint8_t fileCount = 0;

    if (argc <= 1)
    {
        *out_flagCount = 0;
        *out_fileCount = 0;
        *out_flagArgvIndices = NULL;
        *out_fileArgvIndices = NULL;
        printf("blank");
        return;
    }

    fileArgvIndices = (uint8_t*)calloc(argc - 1, sizeof(uint8_t));
    flagArgvIndices = (uint8_t*)calloc(argc - 1, sizeof(uint8_t));

    if (fileArgvIndices == NULL | flagArgvIndices == NULL)
    {
        printf("allocation failed");
        free(fileArgvIndices);
        free(flagArgvIndices);
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (_stricmp(argv[i] + 1, "step") == 0)
            {
                *printMode = STEP;
            }
			else if (_stricmp(argv[i] + 1, "min") == 0)
            {
                *printMode = MINIMAL;
            }
            flagArgvIndices[flagCount++] = (uint8_t)i;
        }
        else
        {
            fileArgvIndices[fileCount++] = (uint8_t)i;
        }
    }

    uint8_t* tmpFlagIndices = NULL;
    uint8_t* tmpFileIndices = NULL;

    if (flagCount > 0) 
    {
        tmpFlagIndices = (uint8_t*)realloc(flagArgvIndices, sizeof(uint8_t) * flagCount);
        if (tmpFlagIndices == NULL)
        {
            printf("resize failed");
            free(flagArgvIndices);
            free(fileArgvIndices);
            exit(1);
        }
    }
    else
    {
        free(flagArgvIndices);
    }

    if (fileCount > 0)
    {
        tmpFileIndices = (uint8_t*)realloc(fileArgvIndices, sizeof(uint8_t) * fileCount);
        if (tmpFileIndices == NULL)
        {
            printf("resize failed");
            free(tmpFlagIndices);
            free(fileArgvIndices);
            exit(1);
        }
    }
    else
    {
        free(fileArgvIndices);
    }

    *out_flagCount = flagCount;
    *out_fileCount = fileCount;
    *out_flagArgvIndices = tmpFlagIndices;
    *out_fileArgvIndices = tmpFileIndices;
}

void setColor(int pos, uint32_t colorByteStart, uint32_t colorByteWidth, const char* color)
{
    if (pos == colorByteStart)
    {
        printf(RED);
    }
    else if ((pos > colorByteStart) && (pos < colorByteStart + colorByteWidth))
    {
        printf(RED);
    }
    else if (pos >= colorByteStart + colorByteWidth)
    {
        printf(RESET);
    }
    
}

void printBuffer(unsigned char* buffer, long size, char* file)
{
    int pos = 0;
    printf("\nfile \"%s\" is %zu bytes:\n\n", file, size);
    printf("  ");
    printf("          ");
    printf("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    int rows = size / 16;
    int lastRowCols = size % 16;
    printf("\n\n");
    for (int i = 0; i < rows; i++)
    {
        printf("  " );
        printf("%08X  ", pos);
        for (int i = 0; i < 16; i++)
        {            
            printf("%02X ", buffer[pos]);
            pos++;
        }
        printf("\n");
    }
    if (lastRowCols > 0)
    {
        printf("  ");
        printf("%08X  ", pos);
        for (int i = 0; i < lastRowCols; i++)
        {            
            printf("%02X ", buffer[pos]);
            pos++;
        }
    }
    printf("\n\n" );
}

void printBufferStepThru(unsigned char* buffer, long size, uint32_t colorByteStart, uint32_t colorByteWidth, char* file)
{
    int pos = 0;
    printf("\nfile \"%s\" is %zu bytes:\n\n  ", file, size);
    printf("byte position" RED " %u\n\n" RESET, colorByteStart);
        
    printf(RESET "            00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n\n");
    int rows = size / 16;
    int lastRowCols = size % 16;    
    for (int i = 0; i < rows && pos < size; i++)
    {
        printf(RESET"  %08X  ", pos);
        for (int j = 0; j < 16 && pos < size; j++)
        {
            setColor(pos, colorByteStart, colorByteWidth, RED);
            printf("%02X ", buffer[pos]);
            pos++;
        }
        printf("\n");
    }
    if (lastRowCols > 0)
    {
        printf(RESET "  %08X  ", pos);
        for (int i = 0; i < lastRowCols && pos < size; i++)
        {
            setColor(pos, colorByteStart, colorByteWidth, RED);
            printf("%02X ", buffer[pos]);
            pos++;
        }
    }
    printf(RESET "\n\n");
}

void listArgs(char** argv, uint8_t* flagArgvIndices, uint8_t* fileArgvIndices, uint8_t flagCount, uint8_t fileCount)
{
    if (flagCount == 0 && fileCount == 0)
    {
        return;
    }

    for (int i = 0; i < flagCount; i++) // list flags
    {
        printf("flag %s\n", argv[flagArgvIndices[i]] + 1); // +1 to skip first char in flag args
    }
    for (int i = 0; i < fileCount; i++) // list files
    {
        printf("file %s\n", argv[fileArgvIndices[i]]);
    }
}

bool openBinary(char* file, uint8_t* validFileCount, long* fileSizes, unsigned char** buffers)
{
    FILE* fp;
    fopen_s(&fp, file, "rb");
    if (fp == NULL)
    {
        printf("; can't open %s\n", file);
        return false;
    }
    else
    {
        printf("; can open %s\n\n", file);        
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (buffer == NULL)
    {
        printf("error creating buffer for %s\n", file);
        fclose(fp);
        return false;
    }
    size_t readBytes = fread(buffer, 1, fileSize, fp);
    if (readBytes != fileSize)
    {
        printf("readBytes != fileSize for file %s\n", file);
        return false;
    }
    fclose(fp);
    fileSizes[*validFileCount] = fileSize;
    buffers[*validFileCount] = buffer;
    (*validFileCount)++;
    return true;
}

void printModed(unsigned char* buffer, long bufferSize, char* file, enum PrintMode mode)
{
    switch (mode)
    {
    case IMMEDIATE:
        printBuffer(buffer, bufferSize, file);
        break;
    case STEP: {
        int pos = 0;
        bool running = true;
        int code = 0;
        while (running)
        {
            system("cls");
            printBufferStepThru(buffer, bufferSize, pos, 1, file);
            int code = _getch();
            if (code == 27 || pos >= bufferSize || bufferSize == 0) // ESC || out of bounds || empty test
            {
                break;
            }
            pos++;
        }
        break;
    }
	case MINIMAL:
		break;
    default:
        break;
    }
}

unsigned char** loadBinaries(char** argv, uint8_t* fileArgvIndices, uint8_t fileCount, long** out_bufferSizes, uint8_t* out_validFileCount, uint8_t** out_validFileIndices)
{
    // iterate over files, identify legitimate file count, identify file sizes.
    // allocate an unsigned char** to store array of pointers to unsigned char arrays.
    // unsigned char** Buffers should be (legitFileCount * sizeof(unsigned char*))
    // Buffers[i] should be (fileSize * sizeof(unsigned char));
    uint8_t validFileCount = 0; //incremented by openBinary();
    long* fileSizes = (long*)calloc(fileCount, sizeof(long));
    unsigned char** buffers = (unsigned char**)calloc(fileCount, sizeof(unsigned char*));
    uint8_t* tmp_validFileIndices = (uint8_t*)calloc(fileCount, sizeof(uint8_t));

    for (int i = 0; i < fileCount; i++)
    {
        if (openBinary(argv[fileArgvIndices[i]], &validFileCount, fileSizes, buffers))
        {
            tmp_validFileIndices[validFileCount-1] = i;
        }
    }

    long* tmp_fileSizes = (long*)realloc(fileSizes, validFileCount * sizeof(long)); 
    unsigned char** tmp_buffers = (unsigned char**)realloc(buffers, validFileCount * sizeof(unsigned char*));
    if (validFileCount != 0 && (tmp_fileSizes == NULL || tmp_buffers == NULL))
    {
        printf("Error reallocating file size or buffer arrays.");
        exit(1);
    }
    *out_bufferSizes = tmp_fileSizes;
    *out_validFileCount = validFileCount;
    *out_validFileIndices = tmp_validFileIndices;
    return tmp_buffers;
}


int main(int argc, char *argv[])
{
    uint8_t* flagArgvIndices;
    uint8_t* fileArgvIndices;
    uint8_t flagCount;
    uint8_t fileCount;

    uint8_t validFileCount = 0;
    uint8_t* validFileIndices;

    unsigned char** buffers;
    long* bufferSizes;

    enum PrintMode printMode = IMMEDIATE;

    parseArgs(argc, argv, &flagArgvIndices, &fileArgvIndices, &flagCount, &fileCount, &printMode);
    buffers = loadBinaries(argv, fileArgvIndices, fileCount, &bufferSizes, &validFileCount, &validFileIndices);

    long position;
    uint64_t packed;    

    printf("bits 16\n\n");
    for (int bufferIndex = 0; bufferIndex < validFileCount; bufferIndex++)
    {        
        position = 0;
        printModed(buffers[bufferIndex], bufferSizes[bufferIndex], argv[fileArgvIndices[validFileIndices[bufferIndex]]], printMode);
               
        while(position < bufferSizes[bufferIndex])
        {
            uint8_t instructionIndex = 0;
            uint8_t bufferInstruction = 0;
            for (; instructionIndex < sizeof(Instructions) / sizeof(Instructions[0]); instructionIndex++)
            {
                bufferInstruction = (uint8_t)(buffers[bufferIndex][position]) >> (8 - Instructions[instructionIndex].instructionBitSize);
                if (Instructions[instructionIndex].instruction == bufferInstruction)
                {
                    position += Instructions[instructionIndex].f_print_instruction(buffers[bufferIndex] + position, bufferSizes[bufferIndex] - position);
                    break;
                }
            }
        } 
    }    
}

