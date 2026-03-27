# Intel 8086 Instruction Decoder

A from-scratch partial implementation of an Intel 8086 instruction decoder written in C-style C++.

## Overview

This project parses raw binary instruction streams and produces human-readable 8086 assembly.

This project was inspired by assignments from ComputerEnhance (by Casey Muratori).
It was built independently without:
 - starter code
 - architectural guidance
 - parsing strategy hints

The goal was to develop an instruction decoding system from scratch using only official Intel 8086 documentation.

## Features
 - Decodes 8086 instruction opcodes from raw binary
 - Handles mod-reg-r/m byte parsing
 - Supports multiple addressing modes
 - Processes variable-length instructions
 - Outputs formatted assembly instructions

## Usage
Flags:
 - (NONE): Display the byte stream within a table-view prior to printing the decoded instructions.
 - `-min`: Hide the byte table, and only print the decoded instructions.
 - `-step` (Incomplete): Display the byte stream within a table-view and highlight the active byte. Interacting with the keyboard will increment the active byte.

## Example 

Input: 
```
  03 18 03 5E 00 83 C6 02 83 C5 02 83 C1 08 03 5E
  00 03 4F 02 02 7A 04 03 7B 06 01 18 01 5E 00 01
  5E 00 01 4F 02 00 7A 04 01 7B 06 80 07 22 83 82
  E8 03 1D 03 46 00 02 00 01 D8 00 E0 05 E8 03 04
  E2 04 09 2B 18 2B 5E 00 83 EE 02 83 ED 02 83 E9
  08 2B 5E 00 2B 4F 02 2A 7A 04 2B 7B 06 29 18 29
  5E 00 29 5E 00 29 4F 02 28 7A 04 29 7B 06 80 2F
  22 83 29 1D 2B 46 00 2A 00 29 D8 28 E0 2D E8 03
  2C E2 2C 09 3B 18 3B 5E 00 83 FE 02 83 FD 02 83
  F9 08 3B 5E 00 3B 4F 02 3A 7A 04 3B 7B 06 39 18
  39 5E 00 39 5E 00 39 4F 02 38 7A 04 39 7B 06 80
  3F 22 83 3E E2 12 1D 3B 46 00 3A 00 39 D8 38 E0
  3D E8 03 3C E2 3C 09 75 02 75 FC 75 FA 75 FC 74
  FE 7C FC 7E FA 72 F8 76 F6 7A F4 70 F2 78 F0 75
  EE 7D EC 7F EA 73 E8 77 E6 7B E4 71 E2 79 E0 E2
  DE E1 DC E0 DA E3 D8
```
Output (with -min flag):
```
bits 16

add bx, [bx + si]
add bx, [bp]
add si, 2
add bp, 2
add cx, 8
add bx, [bp]
add cx, [bx + 2]
add bh, [bp + si + 4]
add di, [bp + di + 6]
add [bx + si], bx
add [bp], bx
add [bp], bx
add [bx + 2], cx
add [bp + si + 4], bh
add [bp + di + 6], di
add byte [bx], 34
add word [bp + si + 1000], 29
add ax, [bp]
add al, [bx + si]
add ax, bx
add al, ah
add ax, 1000
add al, -30
add al, 9
sub bx, [bx + si]
sub bx, [bp]
sub si, 2
sub bp, 2
sub cx, 8
sub bx, [bp]
sub cx, [bx + 2]
sub bh, [bp + si + 4]
sub di, [bp + di + 6]
sub [bx + si], bx
sub [bp], bx
sub [bp], bx
sub [bx + 2], cx
sub [bp + si + 4], bh
sub [bp + di + 6], di
sub byte [bx], 34
sub word [bx + di], 29
sub ax, [bp]
sub al, [bx + si]
sub ax, bx
sub al, ah
sub ax, 1000
sub al, -30
sub al, 9
cmp bx, [bx + si]
cmp bx, [bp]
cmp si, 2
cmp bp, 2
cmp cx, 8
cmp bx, [bp]
cmp cx, [bx + 2]
cmp bh, [bp + si + 4]
cmp di, [bp + di + 6]
cmp [bx + si], bx
cmp [bp], bx
cmp [bp], bx
cmp [bx + 2], cx
cmp [bp + si + 4], bh
cmp [bp + di + 6], di
cmp byte [bx], 34
cmp word [4834], 29
cmp ax, [bp]
cmp al, [bx + si]
cmp ax, bx
cmp al, ah
cmp ax, 1000
cmp al, -30
cmp al, 9
jnz $+4
jnz $-2
jnz $-4
jnz $-2
je $+0
jl $-2
jle $-4
jb $-6
jbe $-8
jp $-10
jo $-12
js $-14
jnz $-16
jnl $-18
jg $-20
jnb $-22
ja $-24
jnp $-26
jno $-28
jns $-30
loop $-32
loopz $-34
loopnz $-36
jcxz $-38
```

## Implementation Details
 - Language: C-style C++
 - Manual bit-level parsing of instruction fields
 - Custom decoding logic for opcode and operand extraction
 - Structured parsing pipeline for instruction handling

## Challenges
 - Reconstructing instruction formats using only the Intel 8086 User's Manual: https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf
 - Handling overlapping opcode patterns
 - Managing variable instruction lengths
 - Designing a clean decoding pipeline without prior structure

## Reference
 - The project was tested using assembly listings from the ComputerEnhance performance-aware programming repo:
   https://github.com/cmuratori/computer_enhance/tree/main/perfaware/part1
 - Primary reference: Intel 8086 Family User's Manual:
   https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf
 - Additional validation: Netwide Assembler:
   https://www.nasm.us/

## Future Improvements
 - Expand instruction coverage
 - Automate NASM testing
 - Add per-instruction live decoding as the interactive byte table is stepped through.
 - Add memory emulation to the step-through mode.

## Validation
Decoder correctness was verified through round-trip assembly:
1. Run the decoder with the `-min` flag to output only assembly
2. Assemble output using Netwide Assembler (NASM)
3. Feed resulting binary back into the decoder
4. Confirm identical decoded output

This ensures the decoder produces structurally correct and reversible instruction sequences.
