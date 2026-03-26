#include <iostream>
#include <stdio.h>
#include <stdlib.h>


void getRegNameAndEAC(char* out, size_t outSize, uint8_t rm, uint8_t w, uint8_t mod, int16_t displacement);
uint16_t f_print_mov_add_sub_cmp_RegOrMemToFromReg_100010(unsigned char* buffer, long remainingBytes);
uint16_t f_print_mov_add_sub_cmp_ImmToRegOrMem_1100011(unsigned char* buffer, long remainingBytes);
uint16_t f_print_mov_ImmToReg_1011(unsigned char* buffer, long remainingBytes);
uint16_t f_print_mov_MemToAcc_1010000(unsigned char* buffer, long remainingBytes);
uint16_t f_print_mov_AccToMem_1010001(unsigned char* buffer, long remainingBytes);
uint16_t f_print_add_sub_cmp_ImmToAcc(unsigned char* buffer, long remainingBytes);
uint16_t f_print_jumps_many(unsigned char* buffer, long remainingBytes);
uint16_t f_print_loops_many(unsigned char* buffer, long remainingBytes);

static uint16_t f_print_completionist_inc_dec_call_jmp_push_1111111(unsigned char* buffer, long remainingbytes); // not yet implemented
