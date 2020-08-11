#pragma once
#include "symbol_table.h"
struct IC_table {
    int size;
    struct IC_entry** entries;
};

struct IC_entry {
    char* key;
    struct SymTab* symbol_table;
    struct BasicBlock** basic_block_list;
    char* blockinfo;
    int n_blocks;
    struct IC_entry* next;

};
struct IC_table* create_IC_table(int table_size);
void IC_table_create_entry(struct IC_table* code_table, char* key);
struct IC_entry* IC_table_get_entry(struct IC_table* code_table, char* key);
