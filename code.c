#include "code.h"
#include "helper.h"
#include "parser.h"
#include <string.h>

void get_dest_code(Parser *parser, TranslatedCode *code) {
  const char *dest = parser->destMnemonic;

  const char *binary = lookup_mnemonic(dest_table, dest);
  if (!binary) {
    print_syntax_error(parser->currentInstruction, parser->typeString, parser->lineNumber, 0,
                       "invalid dest mnemonic \'%s\'", dest);
    parser->errorStatus = true;
  } else {
    print_debug(dbg, "found dest mnemonic \"%s\" in lookup table as \"%s\"\n", dest, binary);
    snprintf(code->dest, sizeof code->comp, "%s", binary);
  }
}

void get_comp_code(Parser *parser, TranslatedCode *code) {
  const char *comp = parser->compMnemonic;
  char *equal_sign = strchr(parser->currentInstruction, '=');
  int pos = 0;
  if (equal_sign) {
    pos = (int)(equal_sign - parser->currentInstruction) + 1;
  }
  const char *binary = lookup_mnemonic(comp_table, comp);
  if (!binary) {
    print_syntax_error(parser->currentInstruction, parser->typeString, parser->lineNumber, pos,
                       "invalid comp mnemonic \"%s\"", comp);
    parser->errorStatus = true;
  } else {
    print_debug(dbg, "found comp mnemonic \"%s\" in lookup table as \"%s\"\n", comp, binary);
    snprintf(code->comp, sizeof code->comp, "%s", binary);
  }
}

void get_jump_code(Parser *parser, TranslatedCode *code) {
  const char *jump = parser->jumpMnemonic;
  char *semicolon = strchr(parser->currentInstruction, ';');
  int pos = 0;
  if (semicolon) {
    pos = (int)(semicolon - parser->currentInstruction) + 1;
  }
  const char *binary = lookup_mnemonic(jump_table, jump);
  if (!binary) {
    print_syntax_error(parser->currentInstruction, parser->typeString, parser->lineNumber, pos,
                       "invalid jump mnemonic \"%s\"", jump);
    parser->errorStatus = true;
  } else {
    print_debug(dbg, "found jump mnemonic \"%s\" in lookup table as \"%s\"\n", jump, binary);
    snprintf(code->jump, sizeof code->comp, "%s", binary);
  }
}
