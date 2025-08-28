#include "strlib.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

bool str_starts_with(const char *str, const char *prefix) { return strncmp(str, prefix, strlen(prefix)) == 0; }

bool str_ends_with(const char *str, const char *suffix) {
  size_t start = strlen(str) - strlen(suffix);
  // pointer arithmetic, str is char[], it decays to pointer to the first char of str
  return strncmp((str + start), suffix, strlen(suffix)) == 0;
}

void str_remove_all_whitespace_inplace(char *str) {
  if (!str) {
    return;
  }
  char *write = str;
  char *read = str;
  while (*read) {
    if (!isspace((unsigned char)*read)) {
      // assign current char in read to write then increment both
      *write++ = *read++;
      continue;
    }
    read++;
  }
  *write = '\0';
}

void str_trim_leading_whitespace_inplace(char *str) {
  if (!str) {
    return;
  }
  char *start = str;
  while (*start && isspace((unsigned char)*start)) {
    start++;
  }
  // include null terminator (+ 1)
  memmove(str, start, strlen(start) + 1);
}

void str_trim_trailing_whitespace_inplace(char *str) {
  if (!str) {
    return;
  }
  char *end = str + strlen(str) - 1;
  while (end >= str && isspace((unsigned char)*end)) {
    end--;
  }
  *(end + 1) = '\0';
}

void str_trim_whitespace_inplace(char *str) {
  if (!str) {
    return;
  }
  str_trim_leading_whitespace_inplace(str);
  str_trim_trailing_whitespace_inplace(str);
}

// char s[12] = "  hey yo  ";

// int main() {
//   void (*f[])(char *) = {str_trim_leading_whitespace_inplace, str_trim_trailing_whitespace_inplace,
//                          str_remove_all_whitespace_inplace};

//   printf("'%s' with len of %llu\n", s, strlen(s));

//   for (size_t i = 0; i < sizeof(f) / sizeof(f[0]); i++) {
//     f[i](s);
//     printf("'%s' with len of %llu\n", s, strlen(s));
//   }
// }
