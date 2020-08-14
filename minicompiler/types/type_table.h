#pragma once
#include "symbol_table.h"
enum TypeType {
    TYPE_BUILTIN,
    TYPE_STRUCT
};

struct TypeTab_entry {
    enum TypeType type;
    char* key;
    unsigned long width;
    union {
        int widening_priority;
        void* struct_symbol;
    };

    struct TypeTab_entry* next;
};

struct TypeTab {
    int table_size;
    struct TypeTab_entry** entries;
};

struct TypeTab* create_TypeTab(int table_size);
struct TypeTab_entry* TypeTab_builtin_pair(char* key, int widening_priority, int width);
struct TypeTab_entry* TypeTab_struct_pair(char* key, struct SymTab* struct_symbol);
void TypeTab_set_builtin(struct TypeTab* type_table, char* key, int widening_priority, int width);

void TypeTab_set(struct TypeTab* type_table, char* key, int widening_priority);
int get_widening_type(struct TypeTab* type_table, char* key);
unsigned long get_type_and_width(struct TypeTab* type_table, char* key);
int TypeTab_check_and_set(struct TypeTab* type_table, char* key, void* struct_symbol);
int TypeTab_check_defined(struct TypeTab* type_table, char* key);
