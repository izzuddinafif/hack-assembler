#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include <stdio.h>
#include <stdlib.h>

int g_status = EXIT_FAILURE;

int main(int argc, char **argv) {
  // initialize debugger
  init_debugger(dbg, true);
  print_debug(dbg, "Heya!\n");

  printf("Welcome to Afif's Hack Assembler!\n\n");
  if (argc < 2 || !str_ends_with(argv[1], ".asm")) {
    printf("Usage: %s <file_name.asm>\n", argv[0]);
    return g_status;
  }
  const char *file_name = argv[1];

  Parser p;
  Parser *parser = &p;
  parser_init(parser, file_name);

  while (has_more_lines(parser)) {
    advance(parser);

    parser->type = instruction_type(parser);

    if (parser->type == A_INSTRUCTION || parser->type == L_INSTRUCTION) {
      get_symbol(parser);
    } else {
      get_dest(parser);
      get_comp(parser);
      get_jump(parser);
    }
  }

  parser_destroy(parser);
  g_status = EXIT_SUCCESS;
  return g_status;
}
