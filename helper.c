#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include "types.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *get_color_for_fd(int fd, const char *code) {
  return isatty(fd) ? code : "";
}

Debugger debugger;
Debugger *dbg = &debugger;

const int MAX_CONSTANT_SIZE = 32767;

void init_debugger(Debugger *d, bool enabled) { d->enabled = enabled; }

// void check_io_error(FILE *file, const char *filename) {
//   if (ferror(file)) {
//     fprintf(stderr, "[ERROR] I/O error on %s: ", filename);
//     perror("");
//     fclose(file);
//     exit(1);
//   } else if (feof(file)) {
//     fprintf(stderr, "End of file reached: %s\n", filename);
//   }
// }

bool line_is_spaces_only_or_empty(const char *string) {
  if (!string)
    return true;
  while (*string) {
    if (isspace((unsigned char)*string) ==
        0) { // cast to unsigned char to avoid UB
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

void print_syntax_error(const char *line, const char *type, int line_number,
                        int position, const char *format, ...) {
  va_list args;
  fputs(get_color_for_fd(fileno(stderr), RED), stderr);
  fprintf(stderr, "%*s^ ", position, "");
  fputs(get_color_for_fd(fileno(stderr), RESET), stderr);
  char new_msg_buf[S128];
  va_start(args, format);
  vsnprintf(new_msg_buf, sizeof new_msg_buf, format, args);
  va_end(args);

  fprintf(stderr,
          "%s[ERROR] Syntax error on line %d, column %d : %s in %s \"%s\"%s\n",
          get_color_for_fd(fileno(stderr), RED), line_number, position + 1,
          new_msg_buf, type, line, get_color_for_fd(fileno(stderr), RESET));
}

bool is_constant(const char *c) {
  while (*c) {
    if (!isdigit((unsigned char)*c++))
      return false;
  }
  return true;
}

const char *is_not_valid_symbol(char *symbol, InstructionType type) {
  bool is_constant_var = is_constant(symbol);
  if (!is_constant_var && isdigit((unsigned char)*symbol)) {
    return symbol; // cant start symbol with a digit
  }
  if (is_constant_var &&
      (type == L_INSTRUCTION || !is_valid_const_size(symbol))) {
    return symbol;
  }
  // DEBUG_LOG(dbg, "checking string \'%s\'.. ", symbol);
  while (*symbol) {
    int sym = (unsigned char)*symbol;
    // DEBUG_LOG(dbg, "checking char \'%c\'.. \n", *symbol);
    if (!(isalnum(sym) || sym == '_' || sym == '.' || sym == '$' ||
          sym == ':')) {
      // DEBUG_LOG(dbg, "%c is NOT a valid symbol\n", sym);
      return symbol;
    }
    symbol++;
  }
  return nullptr;
}

bool is_valid_const_size(const char *string) {
  int result = 0;
  if (str_to_int(string, &result)) {
    // DEBUG_LOG(dbg, "converted %d\n", result);
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
  // DEBUG_LOG(dbg, "checking C instruction \"%s\"\n", instruction);
  while (*instruction) {
    // DEBUG_LOG(dbg, "checking char \'%c\'\n", *instruction);
    if (!(isalpha((unsigned char)(*instruction)) || *instruction == '1' ||
          *instruction == '0' || *instruction == ';' || *instruction == '=' ||
          *instruction == '-' || *instruction == '+' || *instruction == '!' ||
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

void reset_fields(Parser *parser, TranslatedCode *code) {
  if (parser) {
    parser->currentInstruction[0] = '\0'; // set all string buffers to empty
    parser->symbol[0] = '\0';
    parser->jumpMnemonic[0] = '\0';
    parser->compMnemonic[0] = '\0';
    parser->destMnemonic[0] = '\0';
    parser->typeString[0] = '\0';
    parser->type = NO_INSTRUCTION;
    parser->errorStatus = false;
    parser->instructionAddress = 0;
  }

  if (code) {
    code->comp[0] = '\0';
    code->dest[0] = '\0';
    code->jump[0] = '\0';
  }
}

void clean_output(Writer *writer) { writer->output[0] = '\0'; }

void int_str_to_bit_str(const char *in, char *bit, size_t buf_size) {
  if (!in || buf_size < 2) {
    if (bit && buf_size > 0) {
      bit[0] = '\0';
    }
    return;
  }
  bit[0] = '0'; // A-instruction starts with 0
  int integer = 0;
  str_to_int(in, &integer);
  int remainder = 0, result = integer;
  for (int i = (int)buf_size - 2; i > 0; i--) {
    remainder = result % 2;
    result /= 2;

    bit[i] = (remainder ? '1' : '0');

    // DEBUG_LOG(dbg, "result %d remainder %d\n", result, remainder);
  }
  bit[buf_size - 1] = '\0';
}

bool int_to_str(int in, char *buf, size_t buf_size) {
  if (!buf || buf_size < 2) {
    return false;
  }
  char reversed[S32];
  int digit = 0;
  int i = 0;

  while (in > 0) {
    digit = in % 10;
    reversed[i++] = (char)digit + 48;
    in /= 10;
  }
  reversed[i] = '\0';

  int j = i - 1;
  int k = 0;
  while (j >= 0) {
    buf[k++] = reversed[j--];
  }
  buf[k] = '\0';
  return true;
}
