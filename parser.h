#pragma once

#include <stdio.h>
#include <string.h>

enum { S4 = 4, S32 = 32, S64 = 64, S128 = 128, S256 = 256, S512 = 512 };
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
  char destCode[S4];
  char destMnemonic[S64];
  char compCode[S4];
  char compMnemonic[S64];
  char jumpCode[S4];
  char jumpMnemonic[S64];
} Parser;

typedef struct {
  char *mnemonic;
  char *binary;
} MnemonicMap;

const char *lookup_mnemonic(MnemonicMap table[], const char *mnemonic_to_find);

void parser_init(Parser *parser, const char *filename);
void parser_destroy(Parser *parser);
void parser_reset_fields(Parser *parser);

bool has_more_lines(Parser *parser);
bool advance(Parser *parser);
void instruction_type(Parser *parser);
void parse_c_instruction(Parser *parser);
void get_symbol(Parser *parser);

extern MnemonicMap comp_table[];
extern MnemonicMap dest_table[];
extern MnemonicMap jump_table[];
