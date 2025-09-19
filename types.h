#pragma once

#include <stdio.h>

enum { S4 = 4, S8 = 8, S32 = 32, S64 = 64, S128 = 128, S256 = 256, S512 = 512 };
typedef enum { NO_INSTRUCTION, A_INSTRUCTION, C_INTRUCTION, L_INSTRUCTION } InstructionType;
typedef struct {
  FILE *inputFile;
  char currentInstruction[S256];
  bool hasMoreLines;
  int lineNumber;
  bool errorStatus;

  // to be filled
  InstructionType type;
  char typeString[S32];
  char symbol[S128];
  char destMnemonic[S64];
  char compMnemonic[S64];
  char jumpMnemonic[S64];
} Parser;

typedef struct {
  FILE *outputFile;
  char output[17]; // 16 bit output string
} Writer;

typedef struct {
  char *mnemonic;
  char *binary;
} MnemonicMap;

typedef struct {
  char dest[S4];
  char comp[S8];
  char jump[S4];
} TranslatedCode;

// typedef struct {

// }
