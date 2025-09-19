#include "writer.h"
#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include "types.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void writer_init(Writer *writer, const char *output_filename) {
  FILE *file = fopen(output_filename, "w");
  if (!file) {
    fprintf(stderr, "Error opening file '%s': ", output_filename);
    perror("");
    return;
  }

  writer->outputFile = file;
}


void assemble_bits(Parser *parser, TranslatedCode *code, Writer *writer) {
  InstructionType type = parser->type;
  switch (type) {
  case A_INSTRUCTION:
    if (is_constant(parser->symbol)) {
      const size_t buf_size = 17;
      char bit_string[17];
      int_str_to_bit_str(parser->symbol, bit_string, buf_size);
      snprintf(writer->output, sizeof writer->output, "%s", bit_string);
    }
    break;
  case C_INTRUCTION:
    snprintf(writer->output, sizeof writer->output, "111%s%s%s", code->comp, code->dest, code->jump);
    break;
  default:
  }
  print_debug(dbg, "bits assembled: \"%s\"\n", writer->output);
}

void write_output(Writer *writer) {
  if (fprintf(writer->outputFile, "%s\n", writer->output) == EOF) {
    perror("fputs failed");
    return;
  }
}

void writer_destroy(Writer *writer) {
  if (!writer) {
    return;
  }
  if (writer->outputFile) {
    fclose(writer->outputFile);
  }
  writer = nullptr;
}
