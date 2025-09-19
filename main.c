#include "code.h"
#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include "types.h"
#include "writer.h"
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
  char file_name[S128];
  snprintf(file_name, sizeof file_name, "%s", argv[1]);

  Parser p;
  Parser *parser = &p;
  TranslatedCode c;
  TranslatedCode *code = &c;
  Writer w;
  Writer *writer = &w;
  parser_init(parser, file_name);
  file_name[strlen( file_name) - 4] = '\0'; // remove .asm
  char output_name[S128];
  snprintf(output_name, sizeof output_name, "%s.hack", file_name);
  writer_init(writer, output_name);
  bool has_errors = false;
  while (advance(parser)) {
    if (!has_more_lines(parser))
      break;
    instruction_type(parser);

    if (parser->type == A_INSTRUCTION || parser->type == L_INSTRUCTION) {
      get_symbol(parser);
    } else {
      parse_c_instruction(parser, code);
    }
    if (parser->errorStatus) {
      has_errors = true;
      reset_fields(parser, code);
      continue;
    }
    print_debug(dbg, "successfully parsed %s on line %d\n", parser->currentInstruction, parser->lineNumber);
    if (!has_errors) {
      assemble_bits(parser, code, writer);
      if (writer->output[0]) {
        write_output(writer);
        clean_output(writer);
      }
    }
  }

  parser_destroy(parser);
  writer_destroy(writer);
  if (has_errors) {
    fprintf(stderr, "\nAssembly of %s.asm failed because of one or more errors\n", file_name);
    remove(output_name);
    g_status = EXIT_FAILURE;
  } else {
    fprintf(stderr, "\nAssembly of %s.asm successful! check %s\n", file_name, output_name);
    g_status = EXIT_SUCCESS;
  }
  return g_status;
}
