#pragma once

#include "types.h"
#include <stdio.h>
#include <string.h>

const char *lookup_mnemonic(MnemonicMap table[], const char *mnemonic_to_find);

void parser_init(Parser *parser, const char *filename);
void parser_destroy(Parser *parser);

bool has_more_lines(Parser *parser);
bool advance(Parser *parser);
void instruction_type(Parser *parser);
void parse_c_instruction(Parser *parser, TranslatedCode *code);
void get_symbol(Parser *parser);

extern MnemonicMap comp_table[];
extern MnemonicMap dest_table[];
extern MnemonicMap jump_table[];
