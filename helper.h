#pragma once

#include "types.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct {
  bool enabled;
} Debugger;

#define FREE(p)                                                                                                        \
  do {                                                                                                                 \
    free(p);                                                                                                           \
    (p) = nullptr;                                                                                                     \
  } while (0)

#ifdef _WIN32
// Windows: Use ctime_s
static inline void print_debug(Debugger *dbg, const char *format, ...) {
  if (dbg->enabled) {
    time_t now = time(nullptr);
    char time_str[26];
    ctime_s(time_str, sizeof(time_str), &now);
    time_str[24] = '\0'; // remove newline
    printf("[DEBUG] - %s - ", time_str);
    va_list args;
    va_start(args, format);
    vprintf_s(format, args);
    va_end(args);
  }
}
#else
// POSIX: Use ctime_r (or ctime as fallback)
static inline void print_debug(Debugger *dbg, const char *format, ...) {
  if (dbg->enabled) {
    time_t now = time(nullptr);
    char time_str[26];
    char *result = sizeof(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ? ctime_r(&now, time_str) : ctime(&now);
    if (result == nullptr)
      result = "unknown time";
    result[24] = '\0';
    printf("[DEBUG - %s] ", time_str);
    va_list args;
    va_start(args, format);
    vprintf_s(format, args);
    va_end(args);
  }
}
#endif

bool is_constant(const char *c);
const char *is_not_valid_symbol(char *symbol, InstructionType type);
bool is_valid_const_size(const char *string);
const char *is_not_valid_c_instruction(const char *instruction);
void init_debugger(Debugger *debugger, bool enabled);
void check_io_error(FILE *file, const char *filename);
bool line_is_spaces_only_or_empty(const char *string);
void remove_comment_inplace(char *buffer);
void print_syntax_error(const char *line, const char *type, int line_number, int position, const char *format, ...)
    __attribute__((format(printf, 5, 6)));
void reset_fields(Parser *parser, TranslatedCode *code);
void clean_output(Writer *writer);
void int_str_to_bit_str(const char *in, char *bit, size_t buf_size);

extern Debugger debugger;
extern Debugger *dbg;
