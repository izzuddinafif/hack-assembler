#include "code.h"
#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include "symbol.h"
#include "types.h"
#include "writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_status = EXIT_FAILURE;

// First pass scans for labels (L-Instructions) and maps them into symbol table
int first_pass(Parser *parser, SymbolTable *symbol_table, Error *err);
// Second pass scans for A-instructions and C-instructions and writes the output
int second_pass(Parser *parser, TranslatedCode *code, Writer *writer,
                SymbolTable *symbol_table, Error *err);

int main(int argc, char **argv) {
  // initialize debugger
  bool en = false;
  char *env = getenv("DBG");
  if (env && strcmp(env, "y") == 0)
    en = true;
  init_debugger(dbg, en);
  DEBUG_LOG(dbg, "Heya, debug mode is on!");

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
  SymbolTable st;
  SymbolTable *symbol_table = &st;
  Error e;
  Error *err = &e;

  if (!parser_init(parser, file_name)) {
    fprintf(stderr, "Error opening file '%s': ", file_name);
    perror("");
    return g_status;
  }
  file_name[strlen(file_name) - 4] = '\0'; // remove .asm
  char output_name[S128];
  snprintf(output_name, sizeof output_name, "%s.hack", file_name);
  writer_init(writer, output_name);

  if (symbol_init(symbol_table, &err->error_code)) {
    switch (err->error_code) {
    case MEMORY_ERROR:
      printf("%s[ERROR] Insufficent memory%s\n",
             get_color_for_fd(fileno(stderr), RED),
             get_color_for_fd(fileno(stderr), RESET));
      break;
    case DUPLICATE_SYMBOL:
      printf("%s[ERROR] Duplicate predefined label symbol%s\n",
             get_color_for_fd(fileno(stderr), RED),
             get_color_for_fd(fileno(stderr), RESET));
      break;
    default:
    }
  }

  int error_count = first_pass(parser, symbol_table, err);
  reset_fields(parser, nullptr);
  error_count += second_pass(parser, code, writer, symbol_table, err);

  if (error_count > 0) {
    fprintf(stderr, "\n%sAssembly of %s.asm failed with %d error%s%s\n",
            get_color_for_fd(fileno(stderr), MAGENTA), file_name, error_count,
            error_count == 1 ? "" : "s",
            get_color_for_fd(fileno(stderr), RESET));
    remove(output_name);
    g_status = EXIT_FAILURE;
  } else {
    fprintf(
        stderr,
        "\n%sAssembly of %d line%s of %s.asm was successful! check out %s%s\n",
        get_color_for_fd(fileno(stderr), GREEN), parser->lineNumber,
        parser->lineNumber > 1 ? "s" : "", file_name, output_name,
        get_color_for_fd(fileno(stderr), RESET));
    g_status = EXIT_SUCCESS;
  }
  parser_destroy(parser);
  writer_destroy(writer);
  symbol_destroy(symbol_table);
  return g_status;
}

int first_pass(Parser *parser, SymbolTable *symbol_table, Error *err) {
  int error_count = 0;

  while (advance(parser)) {
    instruction_type(parser);
    if (parser->type == L_INSTRUCTION) {
      get_symbol(parser, symbol_table, err);
      if (!add_entry(symbol_table, parser->symbol, parser->instructionAddress,
                     &err->error_code)) {
        error_count++;
        switch (err->error_code) {
        case MEMORY_ERROR:
          // sprintf(err->string, "%s[ERROR] Insufficent memory%s\n",
          //         get_color_for_fd(fileno(stderr), RED),
          //         get_color_for_fd(fileno(stderr), RESET));
          printf("%s[ERROR] Duplicate label symbol \'%s\' "
                 "on line %d%s\n",
                 get_color_for_fd(fileno(stderr), RED), parser->symbol,
                 parser->lineNumber, get_color_for_fd(fileno(stderr), RESET));
          break;
        case DUPLICATE_SYMBOL:
          // sprintf(err->string,
          //         "%s[ERROR] Duplicate label symbol \'%s\' "
          //         "on line %d%s\n",
          //         get_color_for_fd(fileno(stderr), RED), parser->symbol,
          //         parser->lineNumber, get_color_for_fd(fileno(stderr),
          //         RESET));
          printf("%s[ERROR] Duplicate label symbol \'%s\' "
                 "on line %d%s\n",
                 get_color_for_fd(fileno(stderr), RED), parser->symbol,
                 parser->lineNumber, get_color_for_fd(fileno(stderr), RESET));
          break;
        default:
        }
      }
    } else {
      continue;
    }
    if (parser->errorStatus) {
      error_count++;
      reset_fields(parser, nullptr);
      continue;
    }
    DEBUG_LOG(dbg, "succesfully parsed %s label on line %d",
              parser->currentInstruction, parser->lineNumber);
  }

  parser->hasMoreLines = true;
  parser->lineNumber = 0;
  rewind(parser->inputFile);
  return error_count;
}

int second_pass(Parser *parser, TranslatedCode *code, Writer *writer,
                SymbolTable *symbol_table, Error *err) {
  int error_count = 0;
  while (advance(parser)) {
    if (!has_more_lines(parser))
      break;
    instruction_type(parser);
    printf("%s%s%s\n", get_color_for_fd(fileno(stdout), CYAN),
           parser->currentInstruction, get_color_for_fd(fileno(stdout), RESET));
    if (parser->type == A_INSTRUCTION) {
      get_symbol(parser, symbol_table, err);
    } else if (parser->type == L_INSTRUCTION) {
      // handled in first pass
      continue;
    } else {
      parse_c_instruction(parser, code);
    }
    if (parser->errorStatus) {
      error_count++;
      reset_fields(parser, code);
      continue;
    }
    DEBUG_LOG(dbg, "successfully parsed %s on line %d",
              parser->currentInstruction, parser->lineNumber);
    if (error_count == 0) {
      assemble_bits(parser, code, writer, symbol_table);
      if (writer->output[0]) {
        write_output(writer);
        clean_output(writer);
      }
    }
  }
  symbol_table_dump(symbol_table);
  return error_count;
}
