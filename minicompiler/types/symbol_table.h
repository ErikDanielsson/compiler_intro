#pragma once

enum SymbolType {
    CONSTANT,
    VARIABLE,
    TEMPORARY,
    FUNCTION,
};

struct SymTab_entry {
    char* key;
    enum SymbolType type;
    int counter_value;
    long offset;
    void* symbol;
    char live;
    struct SymTab_entry* next;
};

struct SymTab {
    int table_size;
    int n_childs;
    char* name;
    struct SymTab_entry** entries;
    struct SymTab* parent;
    struct SymTab** childs;
};

struct SymTab* create_SymTab(int table_size, struct SymTab* parent, char* name);

void SymTab_append_child(struct SymTab* parent, struct SymTab* child);

struct SymTab_entry* SymTab_pair(char* key, enum SymbolType type,
                                void* symbol, long offset);

int SymTab_check_and_set(struct SymTab* symboltable, char* key,
                    enum SymbolType type, void* symbol, long offset);

struct SymTab_entry* SymTab_getr(struct SymTab* symbol_table, char* key,
                                                enum SymbolType type);

struct SymTab_entry* SymTab_get_typer(struct SymTab* symbol_table, char* key);

int SymTab_get_counter_val(struct SymTab* symbol_table, char* type_names);

void SymTab_destroy(struct SymTab* symboltable);
void SymTab_destroyr(struct SymTab* symbol_table);

void SymTab_dump(struct SymTab* symboltable, int indent);
