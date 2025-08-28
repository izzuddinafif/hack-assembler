#include "helper.h"
#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Debugger debugger;
Debugger *dbg = &debugger;

void init_debugger(Debugger *d, bool enabled) { d->enabled = enabled; }

void check_io_error(FILE *file, const char *filename) {
  if (ferror(file)) {
    fprintf(stderr, "[ERROR] I/O error on %s: ", filename);
    perror("");
    fclose(file);
    exit(1);
  } else if (feof(file)) {
    fprintf(stderr, "End of file reached: %s\n", filename);
  }
}

bool line_is_spaces_only_or_empty(const char *string) {
  if (!string)
    return true;
  while (*string) {
    if (isspace((unsigned char)*string) == 0) { // cast to unsigned char to avoid UB
      // printf("non space char found in %s\n", string);
      return false;
    }
    ++string;
  }
  return true;
}

void remove_comment_inplace(char *buffer) {
  char *result = strstr(buffer, "//");
  if (result) {
    *result = '\0';
  }
}

void print_syntax_error(const char *message, const char *line, InstructionType instruction_type, int line_number,
                        int column) {
  char type[15];
  switch (instruction_type) {
  case A_INSTRUCTION:
    strcpy(type, "A-instruction");
    break;
  case C_INTRUCTION:
    strcpy(type, "C-instruction");
    break;
  case L_INSTRUCTION:
    strcpy(type, "L-instruction");
    break;
  default:
    strcpy(type, "No-instruction");
  }

  for (int i = 0; i < column; i++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "^\n");
  fprintf(stderr, "[ERROR] Syntax error on line %d: %s in %s \"%s\"\n", line_number, message, type, line);
}

// TODO: implement these:
bool is_valid_symbol(char *symbol) {}

bool is_string_numeric(char *string) {}
