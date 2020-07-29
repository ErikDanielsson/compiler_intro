#pragma once

enum SymbolType {
    VARIABLE,
    STRUCTURE,
    FUNCTION,
    TYPE,
};

struct SymTab_entry {
    char* key;
    enum SymbolType type;
    void* symbol;
    struct SymTab_entry* next;
};

struct SymTab {
    int table_size;
    struct SymTab_entry** entries;
    struct SymTab* parent;
};

struct SymTab* create_SymTab();

struct SymTab_entry* SymTab_pair(char* key,
                                enum SymbolType type,
                                void* symbol);

void SymTab_set(struct SymTab* symboltable, char* key,
                    enum SymbolType type, void* symbol);

enum SymbolType SymTab_get(struct SymTab* symboltable, char* key,
                            void** symbol_out);

void SymTab_destroy(struct SymTab* symboltable);

void SymTab_dump(struct SymTab* symboltable);
