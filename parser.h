#pragma once

#include <stdio.h>
#include <string.h>

enum { S4 = 4, S128 = 128, S256 = 256, S512 = 512 };
typedef enum { NO_INSTRUCTION, A_INSTRUCTION, C_INTRUCTION, L_INSTRUCTION } InstructionType;
typedef struct {
  FILE *inputFile;
  char currentInstruction[S256];
  bool hasMoreLines;
  int lineNumber;

  // to be filled
  InstructionType type;
  char symbol[S128];
  char dest[S4];
  char comp[S4];
  char jump[S4];
} Parser;

typedef struct {
  char *mnemonic;
  char *binary;
} MnemonicMap;

const char *lookup_mnemonic(MnemonicMap table[], const char *mnemonic_to_find);
void parser_init(Parser *parser, const char *filename);
void parser_destroy(Parser *parser);

bool has_more_lines(Parser *parser);
void advance(Parser *parser);
InstructionType instruction_type(Parser *parser);
void get_symbol(Parser *parser);
void get_dest(Parser *parser);
void get_comp(Parser *parser);
void get_jump(Parser *parser);
