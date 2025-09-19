#pragma once

#include "parser.h"
#include "types.h"

void get_dest_code(Parser *parser, TranslatedCode *code);
void get_comp_code(Parser *parser, TranslatedCode *code);
void get_jump_code(Parser *parser, TranslatedCode *code);
