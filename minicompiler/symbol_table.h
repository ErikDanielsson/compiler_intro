#pragma once

enum SymbolType {
    VARIABLE,
    STRUCTURE,
    FUNCTION,
};

struct SymTab_entry {
    char* key;
    enum SymbolType type;
    union {
        int widening_priority;
        void* symbol;
    };

    struct SymTab_entry* next;
};

struct SymTab {
    int table_size;
    struct SymTab_entry** entries;
    struct SymTab* parent;
    int n_childs;
    struct SymTab** childs;
};

struct SymTab* create_SymTab(int table_size, struct SymTab* parent);

struct SymTab_entry* SymTab_type_pair(char* key,
                                int widening_priority);

void SymTab_set_type(struct SymTab* symbol_table, char* key, int widening_priority);

int get_widening_type(struct SymTab* symbol_table, char* key);

void SymTab_append_child(struct SymTab* parent, struct SymTab* child);

struct SymTab_entry* SymTab_pair(char* key,
                                enum SymbolType type,
                                void* symbol);

int SymTab_check_and_set(struct SymTab* symboltable, char* key,
                    enum SymbolType type, void* symbol);

void* SymTab_getr(struct SymTab* symbol_table, char* key,
                                                enum SymbolType type);

int SymTab_type_declared(struct SymTab* symbol_table, char* type_names);

void SymTab_destroy(struct SymTab* symboltable);

void SymTab_dump(struct SymTab* symboltable, char* title);
