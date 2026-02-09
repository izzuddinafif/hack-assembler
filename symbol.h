#pragma once

#include "types.h"

bool symbol_init(SymbolTable *symbol_table, ErrorCode *err);
bool add_entry(SymbolTable *symbol_table, const char *name, int address,
               ErrorCode *err);
bool contains(SymbolTable *symbol_table, const char *symbol);
int get_address(SymbolTable *symbol_table, const char *symbol_name);
void symbol_table_dump(SymbolTable *table);
