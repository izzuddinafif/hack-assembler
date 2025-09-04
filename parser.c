#include "parser.h"
#include "helper.h"
#include "strlib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MnemonicMap comp_table[] = {
    {"0", "0101010"},   {"1", "0111111"},   {"-1", "0111010"},  {"D", "0001100"},   {"A", "0110000"},
    {"M", "1110000"},   {"!D", "0001101"},  {"!A", "0110001"},  {"!M", "1110001"},  {"-D", "0001111"},
    {"-A", "0110011"},  {"-M", "1110011"},  {"D+1", "0011111"}, {"A+1", "0110111"}, {"M+1", "1110111"},
    {"D-1", "0001110"}, {"A-1", "0110010"}, {"M-1", "1110010"}, {"D+A", "0000010"}, {"D+M", "1000010"},
    {"D-A", "0010011"}, {"D-M", "1010011"}, {"A-D", "0000111"}, {"M-D", "1000111"}, {"D&A", "0000000"},
    {"D&M", "1000000"}, {"D|A", "0010101"}, {"D|M", "1010101"}, {nullptr, nullptr}};

MnemonicMap dest_table[] = {{"null", "000"}, {"M", "001"},  {"D", "010"},   {"DM", "011"},     {"A", "100"},
                            {"AM", "101"},   {"AD", "110"}, {"ADM", "111"}, {nullptr, nullptr}};

MnemonicMap jump_table[] = {{"null", "000"}, {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"},    {"JLT", "100"},
                            {"JNE", "101"},  {"JLE", "110"}, {"JMP", "111"}, {nullptr, nullptr}};

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

bool advance(Parser *parser) {
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
    return true;
  }
  parser->hasMoreLines = false;
  return false;
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

void _parser_symbol_print_error(int ln, const char *invalid_symbol_ptr, InstructionType type, const char *instruction,
                                const char *symbol, int pos) {
  int invalid_symbol_pos = pos + (int)(strchr(symbol, *invalid_symbol_ptr) - symbol);
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
    _parser_symbol_print_error(ln, invalid_symbol_ptr, type, instruction, a_symbol, 1);
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
    _parser_symbol_print_error(ln, invalid_symbol_ptr, type, instruction, l_symbol, 1);
    parser->errorStatus = true;
  }
}

// TODO: implement these:
void get_dest(Parser *parser) {
  const char *instruction = parser->currentInstruction;
  char dest[S64] = "";
  snprintf(dest, sizeof dest, "%s", instruction);
  char *equal_sign = strchr(dest, '=');

  if (equal_sign) {
    *equal_sign = '\0';
  } else {
    strcpy(dest, "null");
  }
  snprintf(parser->destMnemonic, sizeof parser->destMnemonic, "%s", dest);

  // const char *binary = lookup_mnemonic(dest_table, dest);
  // if (!binary) {
  //   print_syntax_error(instruction, parser->type, parser->lineNumber, (int)strlen(equal_sign),
  //                      "invalid dest mnemonic \'%s\'", dest);
  // } else {
  //   print_debug(dbg, "found dest mnemonic \"%s\" in lookup table as \"%s\"\n", dest, binary);
  //   snprintf(parser->dest, sizeof parser->dest, "%s", binary);
  // }
}

void get_comp(Parser *parser) {
  char comp[S64] = "";
  snprintf(comp, sizeof comp, "%s", parser->currentInstruction);

  char *semicolon = strchr(comp, ';');
  char *equal_sign = strchr(comp, '=');

  if (semicolon) {
    *semicolon = '\0';
  }
  if (equal_sign) {
    memmove(comp, equal_sign + 1, strlen(equal_sign) + 1);
  };
  snprintf(parser->compMnemonic, sizeof parser->compMnemonic, "%s", comp);
  // const char *binary = lookup_mnemonic(comp_table, comp);
  // if (!binary) {
  //   print_syntax_error(instruction, parser->type, parser->lineNumber, (int)strlen(semicolon),
  //                      "invalid comp mnemonic \'%s\'", comp);
  // } else {
  //   print_debug(dbg, "found comp mnemonic \"%s\" in lookup table as \"%s\"\n", comp, binary);
  //   snprintf(parser->comp, sizeof parser->comp, "%s", binary);
  // }
}

void get_jump(Parser *parser) {
  char jump[S64] = "";
  char *semicolon = strchr(parser->currentInstruction, ';');
  if (semicolon) {
    snprintf(jump, sizeof jump, "%s", semicolon + 1);
  } else {
    strcpy(jump, "null");
  };
  snprintf(parser->jumpMnemonic, sizeof parser->jumpMnemonic, "%s", jump);

  // const char *binary = lookup_mnemonic(jump_table, jump);
  // if (!binary) {
  //   print_syntax_error(instruction, parser->type, parser->lineNumber, (int)strlen(semicolon),
  //                      "invalid jump mnemonic \'%s\'", jump);
  // } else {
  //   print_debug(dbg, "found jump mnemonic \"%s\" in lookup table as \"%s\"\n", jump, binary);
  //   snprintf(parser->jump, sizeof parser->jump, "%s", binary);
  // }
}

void parse_c_instruction(Parser *parser) {
  const char *invalid_instr_ptr = is_not_valid_c_instruction(parser->currentInstruction);
  if (!invalid_instr_ptr) {
    get_dest(parser);
    get_jump(parser);
    get_comp(parser);
    return;
  }
  // print_debug(dbg, "found error on %s\n", invalid_instr_ptr);
  int invalid_instr_pos =
      0 + (int)(strchr(parser->currentInstruction, *invalid_instr_ptr) - parser->currentInstruction);
  if (isspace((unsigned char)*invalid_instr_ptr)) {
    print_syntax_error(parser->currentInstruction, parser->type, parser->lineNumber, invalid_instr_pos,
                       "invalid whitespace \'%c\'", *invalid_instr_ptr);
  } else {
    print_syntax_error(parser->currentInstruction, NO_INSTRUCTION, parser->lineNumber, invalid_instr_pos,
                       "invalid char \'%c\'", *invalid_instr_ptr);
  }
  parser->errorStatus = true;
}
