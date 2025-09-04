#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Text colors
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

// Bright colors
#define BRIGHT_RED "\033[91m"
#define BRIGHT_GREEN "\033[92m"
#define BRIGHT_YELLOW "\033[93m"

// Background colors
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"

// Styles
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define RESET "\033[0m"

const char *get_color_for_fd(int fd, const char *code) { return isatty(fd) ? code : ""; }

Debugger debugger;
Debugger *dbg = &debugger;

const int MAX_CONSTANT_SIZE = 32767;

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

void print_syntax_error(const char *line, InstructionType instruction_type, int line_number, int position,
                        const char *format, ...) {
  va_list args;
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
  fputs(get_color_for_fd(fileno(stderr), RED), stderr);
  fprintf(stderr, "%*s^\n", position, "");
  fputs(get_color_for_fd(fileno(stderr), RESET), stderr);
  char new_msg_buf[S128];
  va_start(args, format);
  vsnprintf(new_msg_buf, sizeof new_msg_buf, format, args);
  va_end(args);

  fprintf(stderr, "%s[ERROR]%s Syntax error on line %d, column %d : %s in %s \"%s\"\n",
          get_color_for_fd(fileno(stderr), RED), get_color_for_fd(fileno(stderr), RESET), line_number, 1 + position,
          new_msg_buf, type, line);
}

bool is_constant(const char *c) {
  while (*c) {
    if (!isdigit((unsigned char)*c++))
      return false;
  }
  return true;
}

// returns nullptr if valid or the invalid symbol's pointer otherwise
const char *is_not_valid_symbol(char *symbol, InstructionType type) {
  bool is_constant_var = is_constant(symbol);
  if (!is_constant_var && isdigit((unsigned char)*symbol)) {
    return symbol; // cant start with a digit
  }
  if (is_constant_var && (type == L_INSTRUCTION || !is_valid_const_size(symbol))) {
    return symbol;
  }
  // print_debug(dbg, "checking string \'%s\'.. \n", symbol);
  while (*symbol) {
    int sym = (unsigned char)*symbol;
    // print_debug(dbg, "checking char \'%c\'.. \n", *symbol);
    if (!(isalnum(sym) || sym == '_' || sym == '.' || sym == '$' || sym == ':')) {
      // print_debug(dbg, "%c is NOT a valid symbol\n", sym);
      return symbol;
    }
    symbol++;
  }
  return nullptr;
}

bool is_valid_const_size(const char *string) {
  int result = 0;
  if (str_to_int(string, &result)) {
    // print_debug(dbg, "converted %d\n", result);
    if (result > MAX_CONSTANT_SIZE) {
      return false;
    }
  } else {
    printf("conversion to int failed\n");
    exit(1);
  }
  return true;
}

const char *is_not_valid_c_instruction(const char *instruction) {
  const char *equal_sign = strchr(instruction, '=');
  const char *semicolon = strchr(instruction, ';');
  const char *start = instruction;
  // print_debug(dbg, "checking C instruction \"%s\"\n", instruction);
  while (*instruction) {
    // print_debug(dbg, "checking char \'%c\'\n", *instruction);
    if (!(isalpha((unsigned char)(*instruction)) || *instruction == '1' || *instruction == '0' || *instruction == ';' ||
          *instruction == '=' || *instruction == '-' || *instruction == '+' || *instruction == '!' ||
          *instruction == '&' || *instruction == '|')) {
      return instruction;
    }
    instruction++;
  }
  if (equal_sign) {
    if (equal_sign - start == 0 || strlen(equal_sign) == 1) {
      return equal_sign;
    }
  }
  if (semicolon) {
    if (strlen(semicolon) == 1 || semicolon - start == 0) {
      return semicolon;
    }
  }
  return nullptr;
}
