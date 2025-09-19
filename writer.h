#pragma once

#include "types.h"

void writer_init(Writer *writer, const char *output_filename);
void assemble_bits(Parser *parser, TranslatedCode *code, Writer *writer);
void write_output(Writer *writer);
void writer_destroy(Writer *writer);
