#include "symbol.h"
#include "helper.h"
#include "types.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

bool symbol_init(SymbolTable *symbol_table, ErrorCode *err) {

  Symbol predefined_symbols[] = {// Registers R0-R15
                                 {"R0", 0},
                                 {"R1", 1},
                                 {"R2", 2},
                                 {"R3", 3},
                                 {"R4", 4},
                                 {"R5", 5},
                                 {"R6", 6},
                                 {"R7", 7},
                                 {"R8", 8},
                                 {"R9", 9},
                                 {"R10", 10},
                                 {"R11", 11},
                                 {"R12", 12},
                                 {"R13", 13},
                                 {"R14", 14},
                                 {"R15", 15},

                                 // Special pointers
                                 {"SP", 0},
                                 {"LCL", 1},
                                 {"ARG", 2},
                                 {"THIS", 3},
                                 {"THAT", 4},

                                 // I/O pointers
                                 {"SCREEN", 16384},
                                 {"KBD", 24576}};

  const int NUM_PREDEFINED =
      sizeof(predefined_symbols) / sizeof(predefined_symbols[0]);

  symbol_table->size = 0;
  symbol_table->capacity = S32;
  symbol_table->entries =
      malloc((size_t)symbol_table->capacity * sizeof(Symbol));
  if (!symbol_table->entries) {
    *err = MEMORY_ERROR;

    return false;
  }
  symbol_table->next_var_address = 16; // addresses 0-15 are preoccupied

  for (int i = 0; i < NUM_PREDEFINED; i++) {
    if (!add_entry(symbol_table, predefined_symbols[i].name,
                   predefined_symbols[i].address, err)) {
      return false;
    }
  }

  *err = SUCCESS;
  return true;
}

bool add_entry(SymbolTable *symbol_table, const char *name, int address,
               ErrorCode *err) {
  if (symbol_table->size + 1 >= symbol_table->capacity) {
    symbol_table->capacity *= 2;
    Symbol *new_buf = realloc(symbol_table->entries,
                              (size_t)symbol_table->capacity * sizeof(Symbol));
    if (!new_buf) {
      *err = MEMORY_ERROR;
      return false;
    }
    symbol_table->entries = new_buf;
  }
  // check if sym exists first
  if (contains(symbol_table, name)) {
    *err = DUPLICATE_SYMBOL;
    return false;
  }
  int size = symbol_table->size;
  symbol_table->entries[size].name = malloc(strlen(name) + 1);
  if (!symbol_table->entries[size].name) {
    *err = MEMORY_ERROR;
    return false;
  }
  strcpy(symbol_table->entries[size].name, name);
  symbol_table->entries[size].address = address;
  DEBUG_LOG(dbg, "successfully added symbol entry: \'%s\' with address %d",
            symbol_table->entries[size].name,
            symbol_table->entries[size].address);
  symbol_table->size++;
  *err = SUCCESS;
  return true;
}

bool contains(SymbolTable *symbol_table, const char *symbol_name) {
  for (int i = 0; i < symbol_table->size; i++) {
    if (strcmp(symbol_name, symbol_table->entries[i].name) == 0)
      return true;
  }
  return false;
}

int get_address(SymbolTable *symbol_table, const char *symbol_name) {
  for (int i = 0; i < symbol_table->size; i++) {
    if (strcmp(symbol_name, symbol_table->entries[i].name) == 0)
      return symbol_table->entries[i].address;
  }
  return -1;
}

void symbol_table_dump(SymbolTable *table) {
  DEBUG_LOG(dbg, "=== Symbol Table Dump (%d/%d entries) ===", table->size,
            table->capacity);
  for (int i = 0; i < table->size; i++) {
    DEBUG_LOG(dbg, "  [%d] %-15s -> %d", i, table->entries[i].name,
              table->entries[i].address);
  }
  DEBUG_LOG(dbg, "Next var address: %d", table->next_var_address);
}
