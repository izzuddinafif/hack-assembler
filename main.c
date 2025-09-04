#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_status = EXIT_FAILURE;

int main(int argc, char **argv) {
  // initialize debugger
  bool en = false;
  char *env = getenv("DBG");
  if (env && strcmp(env, "y") == 0)
    en = true;
  init_debugger(dbg, en);
  print_debug(dbg, "Heya, debug mode is on!\n");

  printf("Welcome to Afif's Hack Assembler!\n\n");
  if (argc < 2 || !str_ends_with(argv[1], ".asm")) {
    printf("Usage: %s <file_name.asm>\n", argv[0]);
    return g_status;
  }
  const char *file_name = argv[1];

  Parser p;
  Parser *parser = &p;
  parser_init(parser, file_name);

  while (advance(parser)) {
    if (!has_more_lines(parser))
      break;
    parser->type = instruction_type(parser);

    if (parser->type == A_INSTRUCTION || parser->type == L_INSTRUCTION) {
      get_symbol(parser);
    } else {
      parse_c_instruction(parser);
    }
    parser->errorStatus ? (void)0
                        : print_debug(dbg, "parsed instruction: %s on line %d with no error\n",
                                      parser->currentInstruction, parser->lineNumber);
    parser->errorStatus = false;
  }

  parser_destroy(parser);
  g_status = EXIT_SUCCESS;
  return g_status;
}
