#include "writer.h"
#include "helper.h"
#include "parser.h"
#include "strlib.h"
#include "symbol.h"
#include "types.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void writer_init(Writer *writer, FILE *output_file, bool is_logisim_raw) {
  writer->outputFile = output_file;
  writer->output[0] = '\0';
  writer->is_logisim_raw = is_logisim_raw;

  if (is_logisim_raw) {
    fprintf(output_file, "%s\n", "v2.0 raw");
  }
}

void assemble_bits(Parser *parser, TranslatedCode *code, Writer *writer,
                   SymbolTable *symbol_table) {
  InstructionType type = parser->type;
  switch (type) {
  case A_INSTRUCTION:
    if (is_constant(parser->symbol)) {
      const size_t buf_size = 17;
      char bit_string[17];
      int_str_to_bit_str(parser->symbol, bit_string, buf_size);
      DEBUG_LOG(dbg, "assembled integer constant: %s", bit_string);
      snprintf(writer->output, sizeof writer->output, "%s", bit_string);
    } else {

      const size_t buf_size = 17;
      char bit_string[17];
      int address = get_address(symbol_table, parser->symbol);
      DEBUG_LOG(dbg, "got address of %s: %d", parser->symbol, address);
      char addr[15];
      int_to_str(address, addr, sizeof addr);
      int_str_to_bit_str(addr, bit_string, buf_size);
      snprintf(writer->output, sizeof writer->output, "%s", bit_string);
    }
    break;
  case C_INTRUCTION:
    snprintf(writer->output, sizeof writer->output, "111%s%s%s", code->comp,
             code->dest, code->jump);
    break;
  default:
  }
  DEBUG_LOG(dbg, "bits assembled: \"%s\"", writer->output);
}

void write_output(Writer *writer, LogisimWriter *logisim_writer) {
  if (writer->is_logisim_raw) {
    if (fprintf(writer->outputFile, "%s", writer->output) == EOF) {
      perror("fprintf failed");
      return;
    }
    logisim_writer->current_word_count++;
    if (logisim_writer->current_word_count == logisim_writer->words_per_line) {
      if (fprintf(writer->outputFile, "%s", "\n") == EOF) {
        perror("fprintf failed");
        return;
      }
      logisim_writer->current_word_count = 0;
    } else {
      if (fprintf(writer->outputFile, "%s", " ") == EOF) {
        perror("fprintf failed");
        return;
      }
    }
  } else {
    if (fprintf(writer->outputFile, "%s\n", writer->output) == EOF) {
      perror("fprintf failed");
      return;
    }
  }
}

void writer_destroy(Writer *writer) {
  if (!writer) {
    return;
  }
  if (writer->outputFile) {
    fclose(writer->outputFile);
    writer->outputFile = nullptr;
  }
}
