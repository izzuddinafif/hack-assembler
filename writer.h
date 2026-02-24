#pragma once

#include "types.h"

void writer_init(Writer *writer, FILE *output_file, bool is_logisim_raw);
void assemble_bits(Parser *parser, TranslatedCode *code, Writer *writer,
                   SymbolTable *symbol_table);
void write_output(Writer *writer, LogisimWriter *logisim_writer);
void writer_destroy(Writer *writer);
