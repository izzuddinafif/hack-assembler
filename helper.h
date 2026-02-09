#pragma once

#include "types.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

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

const char *get_color_for_fd(int fd, const char *code);

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

typedef struct {
  bool enabled;
} Debugger;

#if defined(__GNUC__) || defined(__clang__)
#define PRINTF_LIKE(fmt_idx, first_arg)                                        \
  __attribute__((format(printf, fmt_idx, first_arg)))
#else
#define PRINTF_LIKE(fmt_idx, first_arg)
#endif

static inline void debug_get_time(char *buf, size_t size) {
#ifdef _WIN32
  SYSTEMTIME st;
  GetLocalTime(&st);

  snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth,
           st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
  struct timespec ts;
  struct tm tm;

  clock_gettime(CLOCK_REALTIME, &ts);
  localtime_r(&ts.tv_sec, &tm);

  strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
  snprintf(buf + strlen(buf), size - strlen(buf), ".%03ld",
           ts.tv_nsec / 1000000);
#endif
}

// #define DEBUG
#ifdef DEBUG

static void debug_vprint(const char *file, int line, const char *func,
                         const char *fmt, va_list args) PRINTF_LIKE(4, 0)
    __attribute__((unused));

static void debug_print(const char *file, int line, const char *func,
                        const char *fmt, ...) PRINTF_LIKE(4, 5)
    __attribute__((unused));

static void debug_vprint(const char *file, int line, const char *func,
                         const char *fmt, va_list args) {
  char timebuf[64];
  debug_get_time(timebuf, sizeof(timebuf));

  fprintf(stderr, "%s[DEBUG] %s | %s:%d (%s): %s",
          get_color_for_fd(fileno(stdout), BLUE), timebuf, file, line, func,
          get_color_for_fd(fileno(stdout), RESET));

  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
}

static void debug_print(const char *file, int line, const char *func,
                        const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  debug_vprint(file, line, func, fmt, args);
  va_end(args);
}

#define DEBUG_LOG(dbg, fmt, ...)                                               \
  do {                                                                         \
    if ((dbg) && (dbg)->enabled)                                               \
      debug_print(__FILE__, __LINE__, __func__,                                \
                  fmt __VA_OPT__(, ) __VA_ARGS__);                             \
  } while (0)

#else
#define DEBUG_LOG(...) ((void)0)
#endif

// !WARNING!
// Use it only like this:
// int *ptr = malloc(100);
// FREE(ptr); // OK
//
// NOT like this:
// FREE(array[i++]); // !DON'T DO THIS!
#define FREE(p)                                                                \
  do {                                                                         \
    free(p);                                                                   \
    (p) = nullptr;                                                             \
  } while (0)

bool is_constant(const char *c);
const char *is_not_valid_symbol(char *symbol, InstructionType type);
bool is_valid_const_size(const char *string);
const char *is_not_valid_c_instruction(const char *instruction);
void init_debugger(Debugger *debugger, bool enabled);
void check_io_error(FILE *file, const char *filename);
bool line_is_spaces_only_or_empty(const char *string);
void remove_comment_inplace(char *buffer);
void print_syntax_error(const char *line, const char *type, int line_number,
                        int position, const char *format, ...)
    __attribute__((format(printf, 5, 6)));
void reset_fields(Parser *parser, TranslatedCode *code);
void clean_output(Writer *writer);
void int_str_to_bit_str(const char *in, char *bit, size_t buf_size);
bool int_to_str(int in, char *buf, size_t buf_size);

extern Debugger debugger;
extern Debugger *dbg;
