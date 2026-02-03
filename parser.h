#pragma once

#include "types.h"
#include <stdio.h>
#include <string.h>

const char *lookup_mnemonic(const MnemonicMap *table, const char *mnemonic_to_find);

bool parser_init(Parser *parser, const char *filename);
void parser_destroy(Parser *parser);

bool has_more_lines(Parser *parser);
bool advance(Parser *parser);
void instruction_type(Parser *parser);
void parse_c_instruction(Parser *parser, TranslatedCode *code);
void get_symbol(Parser *parser);

extern const MnemonicMap comp_table[];
extern const MnemonicMap dest_table[];
extern const MnemonicMap jump_table[];
