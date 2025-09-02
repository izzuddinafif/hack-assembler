#include "parser.h"
#include "helper.h"
#include "strlib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    strncpy(parser->currentInstruction, line_buf, (sizeof parser->currentInstruction) - 1);
    // null terminate in case of input size >= buffer size
    parser->currentInstruction[(sizeof parser->currentInstruction) - 1] = '\0';
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

void _parser_print_error(int ln, const char *invalid_symbol_ptr, InstructionType type, const char *instruction, const char *symbol) {
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
      exit(1);
    }
    
    // skip the @, copy the rest
    char *a_symbol = instruction + 1;
    char *invalid_symbol_ptr = is_not_valid_symbol(a_symbol);
    if (!invalid_symbol_ptr) {
      snprintf(parser->symbol, sizeof parser->symbol, "%s", a_symbol);
      return;
    }
    _parser_print_error(ln, invalid_symbol_ptr, type, instruction, a_symbol);

  } else if (type == L_INSTRUCTION) {
    char *closing_paren = strchr(instruction, ')');
    if (!closing_paren) {
      print_syntax_error(instruction, type, ln, (int)strlen(instruction), "missing ')'");
      exit(1);
    }

    // remove parentheses
    char *l_symbol = instruction + 1;
    l_symbol[strlen(l_symbol) - 1] = '\0';

    char *invalid_symbol_ptr = is_not_valid_symbol(l_symbol);
    if (!invalid_symbol_ptr) {
      snprintf(parser->symbol, sizeof parser->symbol, "%s", l_symbol);
      return;
    }
    _parser_print_error(ln, invalid_symbol_ptr, type, instruction, l_symbol);
    exit(1);
  }
}


// TODO: implement these:
void get_dest(Parser *parser) {}

void get_comp(Parser *parser) {}

void get_jump(Parser *parser) {}
