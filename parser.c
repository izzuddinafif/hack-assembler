#include "parser.h"
#include "helper.h"
#include "strlib.h"
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
    str_trim_whitespace_inplace(line_buf);
    remove_comment_inplace(line_buf);

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

void get_symbol(Parser *parser) {
  InstructionType type = parser->type;
  char *instruction = parser->currentInstruction;
  char *symbol = parser->symbol;
  int ln = parser->lineNumber;

  if (type == A_INSTRUCTION) {
    if (strlen(instruction) < 2) {
      print_syntax_error("missing symbol after @", instruction, type, ln, (int)strlen(instruction));
      exit(1);
    }
    // skip the @, copy the rest
    strncpy(symbol, (instruction + 1), (sizeof *symbol) - 1);
    symbol[(sizeof *symbol) - 1] = '\0';
    // TODO: Implement symbol/numerical value validation (0-32767)

  } else if (type == L_INSTRUCTION) {
    char *closing_paren = strchr(instruction, ')');
    if (!closing_paren) {
      print_syntax_error("missing ')'", instruction, type, ln, (int)strlen(instruction));
      exit(1);
    }
    // TODO: implement symbol extraction and validation
  }
}

// TODO: implement these:
void get_dest(Parser *parser) {}

void get_comp(Parser *parser) {}

void get_jump(Parser *parser) {}
