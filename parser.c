#include "parser.h"
#include "helper.h"
#include "strlib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MnemonicMap comp_table[] = {{"0", "0101010"},   {"1", "0111111"},   {"-1", "0111010"},  {"D", "0001100"},
                            {"A", "0110000"},   {"M", "1110000"},   {"!D", "0001101"},  {"!A", "0110001"},
                            {"!M", "1110001"},  {"-D", "0001111"},  {"-A", "0110011"},  {"-M", "1110011"},
                            {"D+1", "0011111"}, {"A+1", "0110111"}, {"M+1", "1110111"}, {"D-1", "0001110"},
                            {"A-1", "0110010"}, {"M-1", "1110010"}, {"D+A", "0000010"}, {"D+M", "1000010"},
                            {"D-A", "0010011"}, {"D-M", "1010011"}, {"A-D", "0000111"}, {"M-D", "1000111"},
                            {"D&A", "0000000"}, {"D&M", "1000000"}, {"D|A", "0010101"}, {"D|M", "1010101"}};

MnemonicMap dest_table[] = {{"", "000"},  {"M", "001"},  {"D", "010"},  {"DM", "011"},
                            {"A", "100"}, {"AM", "101"}, {"AD", "110"}, {"ADM", "111"}};

MnemonicMap jump_table[] = {{"", "000"},    {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"},
                            {"JLT", "100"}, {"JNE", "101"}, {"JLE", "110"}, {"JMP", "111"}};

const char *lookup_mnemonic(MnemonicMap table[], const char *mnemonic_to_find) {
  for (int i = 0; table[i].mnemonic; i++) {
    if (strcmp(table[i].mnemonic, mnemonic_to_find) == 0) {
      return table[i].binary;
    }
  }
  return nullptr;
}

void parser_init(Parser *parser, const char *Filename) {
  FILE *file = fopen(Filename, "r");
  if (!file) {
    fprintf(stderr, "Error opening file '%s': ", Filename);
    perror("");
    return;
  }

  // init all fields to known values
  parser->inputFile = file;
  parser->type = NO_INSTRUCTION;
  parser->hasMoreLines = true; // assume there are lines initially
  parser->lineNumber = 0;
  parser->errorStatus = false;

  parser->currentInstruction[0] = '\0'; // set all string buffers to empty
  parser->symbol[0] = '\0';
  parser->dest[0] = '\0';
  parser->comp[0] = '\0';
  parser->jump[0] = '\0';

  return;
}

void parser_destroy(Parser *parser) {
  if (!parser)
    return;
  if (!(parser->inputFile)) {
    fclose(parser->inputFile);
  }
  parser = nullptr;
}

bool has_more_lines(Parser *parser) { return parser->hasMoreLines; }

void advance(Parser *parser) {
  char line_buf[S512];
  while (fgets(line_buf, sizeof line_buf, parser->inputFile)) {
    parser->lineNumber++;

    remove_comment_inplace(line_buf);
    str_trim_whitespace_inplace(line_buf);

    if (!*line_buf) {
      continue; // skip comment or empty line
    }

    puts(line_buf);
    snprintf(parser->currentInstruction, sizeof parser->currentInstruction, "%s", line_buf);
    return;
  }
  parser->hasMoreLines = false;
}

InstructionType instruction_type(Parser *parser) {
  char *instruction = parser->currentInstruction;

  if (str_starts_with(instruction, "@"))
    return A_INSTRUCTION;
  else if (str_starts_with(instruction, "("))
    return L_INSTRUCTION;
  else
    return C_INTRUCTION;
}

void _parser_print_error(int ln, const char *invalid_symbol_ptr, InstructionType type, const char *instruction,
                         const char *symbol) {
  int invalid_symbol_pos = 1 + (int)(strchr(symbol, *invalid_symbol_ptr) - symbol);
  if (isdigit((unsigned char)*invalid_symbol_ptr) && invalid_symbol_pos == 1) {
    print_syntax_error(instruction, type, ln, invalid_symbol_pos, "symbol can't start with digit \'%c\'",
                       *invalid_symbol_ptr);
  } else if (isspace((unsigned char)*invalid_symbol_ptr)) {
    print_syntax_error(instruction, type, ln, invalid_symbol_pos, "invalid whitespace \'%c\'", *invalid_symbol_ptr);
  } else {
    print_syntax_error(instruction, type, ln, invalid_symbol_pos, "invalid symbol \'%c\'", *invalid_symbol_ptr);
  }
}

void get_symbol(Parser *parser) {
  InstructionType type = parser->type;
  char *instruction = parser->currentInstruction;
  int ln = parser->lineNumber;

  if (type == A_INSTRUCTION) {
    if (strlen(instruction) < 2) {
      print_syntax_error(instruction, type, ln, (int)strlen(instruction), "missing symbol after @");
      parser->errorStatus = true;
    }

    // skip the @, copy the rest
    int len = (int)strlen(instruction);
    char a_symbol[len]; // - 1 from actual instruction
    snprintf(a_symbol, sizeof a_symbol, "%s", instruction + 1);

    const char *invalid_symbol_ptr = is_not_valid_symbol(a_symbol, type);
    if (!invalid_symbol_ptr) {
      snprintf(parser->symbol, sizeof parser->symbol, "%s", a_symbol);
      return;
    }
    _parser_print_error(ln, invalid_symbol_ptr, type, instruction, a_symbol);
    parser->errorStatus = true;

  } else if (type == L_INSTRUCTION) {
    char *closing_paren = strchr(instruction, ')');
    if (!closing_paren) {
      print_syntax_error(instruction, type, ln, (int)strlen(instruction), "missing ')'");
      parser->errorStatus = true;
    }

    // remove parentheses
    int len = (int)strlen(instruction);
    char l_symbol[len - 1]; // -2 from actual instruction
    snprintf(l_symbol, sizeof l_symbol, "%.*s", len - 2, instruction + 1);
    const char *invalid_symbol_ptr = is_not_valid_symbol(l_symbol, type);
    if (!invalid_symbol_ptr) {
      snprintf(parser->symbol, sizeof parser->symbol, "%s", l_symbol);
      return;
    }
    _parser_print_error(ln, invalid_symbol_ptr, type, instruction, l_symbol);
    parser->errorStatus = true;
  }
}

// TODO: implement these:
void get_dest(Parser *parser) {
}

void get_comp(Parser *parser) {}

void get_jump(Parser *parser) {}
