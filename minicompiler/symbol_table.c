#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "hashing.h"

struct SymTab* create_SymTab(int table_size, struct SymTab* parent)
{
    struct SymTab* symbol_table = malloc(sizeof(struct SymTab));
    symbol_table->table_size = table_size;
    symbol_table->entries = malloc(sizeof(struct SymTab_entry*) * table_size);

    for (int i = 0; i < table_size; i++) {
        symbol_table->entries[i] = NULL;
    }
    symbol_table->parent = parent;
    symbol_table->n_childs = 0;
    return symbol_table;
}

void SymTab_append_child(struct SymTab* parent, struct SymTab* child)
{
    struct SymTab* tmp[parent->n_childs];
    memcpy(tmp, parent->childs, parent->n_childs);
    int n_childs = ++parent->n_childs;
    parent->childs = malloc(sizeof(struct SymTab*)*n_childs);
    memcpy(tmp, parent->childs, n_childs-1);
    parent->childs[n_childs-1] = child;
}

struct SymTab_entry* SymTab_pair(char* key,
                                enum SymbolType type,
                                void* symbol)
{
    struct SymTab_entry* entry = malloc(sizeof(struct SymTab_entry) * 1);
    entry->key = malloc(sizeof(char)*strlen(key)+1);
    strcpy(entry->key, key);
    entry->type = type;
    entry->next = NULL;
    entry->symbol = symbol;
    return entry;
}

struct SymTab_entry* SymTab_type_pair(char* key,
                                int widening_priority)
{
    struct SymTab_entry* entry = malloc(sizeof(struct SymTab_entry));
    entry->key = malloc(sizeof(char)*strlen(key)+1);
    strcpy(entry->key, key);
    entry->widening_priority = widening_priority;
    entry->next = NULL;
    entry->type = STRUCTURE;
    return entry;
}

void SymTab_set_type(struct SymTab* symbol_table, char* key, int widening_priority)
{
    unsigned int slot = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry = symbol_table->entries[slot];
    struct SymTab_entry* prev;
    if (entry == NULL) {
        symbol_table->entries[slot] = SymTab_type_pair(key, type, symbol);
        return 0;
    }
    while (entry != NULL) {
        prev = entry;
        entry = prev->next;
    }
    prev->next = SymTab_type_pair(key, type, symbol);
    return 0;
}

int get_widening_type(struct SymTab* symbol_table, char* key)
{
    unsigned int hashv = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry = symbol_table->entries[hashv];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            *symbol_out = entry->symbol;
            return entry->widening_priority;

        }
        entry = entry->next;
    }
    return -1;
}

int SymTab_check_and_set(struct SymTab* symbol_table, char* key,
                    enum SymbolType type, void* symbol)
{
    unsigned int slot = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry = symbol_table->entries[slot];
    struct SymTab_entry* prev;
    if (entry == NULL) {
        symbol_table->entries[slot] = SymTab_pair(key, type, symbol);
        return 0;
    }
    while (entry != NULL) {

        if (entry->type == type && strcmp(entry->key, key) == 0) {
            printf("Equal\n");
            return 1;
        }
        prev = entry;
        entry = prev->next;
    }
    prev->next = SymTab_pair(key, type, symbol);
    return 0;
}

enum SymbolType find_in_table(struct SymTab* symbol_table, unsigned int hashv, char* key,
                                    void** symbol_out)
{
    struct SymTab_entry* entry = symbol_table->entries[hashv];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            *symbol_out = entry->symbol;
            return entry->type;

        }
        entry = entry->next;
    }
    return -1;
}

/*
 * Search symbol table and parents recursivly for symbol
 */
enum SymbolType SymTab_getr(struct SymTab* symbol_table, char* key,
                            void **symbol_out)
{
    unsigned int hashv = hash(key, symbol_table->table_size);
    enum SymbolType type;
    for(; symbol_table != NULL; symbol_table = symbol_table->parent) {
        if ((type = find_in_table(symbol_table,  hashv, key, symbol_out)) != -1)
            return type;
    }
    (*symbol_out) = NULL;
    return -1;
}

int SymTab_type_declared(struct SymTab* symbol_table, char* key)
{
    unsigned int slot = hash(key, symbol_table->table_size);
    for(; symbol_table != NULL; symbol_table = symbol_table->parent) {
        struct SymTab_entry* entry = symbol_table->entries[slot];
        while (entry != NULL) {
            if (entry->type == STRUCTURE && strcmp(entry->key, key) == 0)
                return 1;
            entry = entry->next;
        }
    }
    return 0;
}


void SymTab_destroy(struct SymTab* symbol_table)
{
    if (symbol_table == NULL)
        return;
    if (symbol_table->entries != NULL) {
        size_t i = 0;
        while (i < symbol_table->table_size) {
            struct SymTab_entry* head = symbol_table->entries[i];
            while (head != NULL) {
                struct SymTab_entry* tmp = head;
                if (tmp->key != NULL) {
                    free(tmp->key);
                    tmp->key = NULL;
                }

                head = head->next;
                free(tmp);
            }
            i++;
        }
        free(symbol_table->entries);
        symbol_table->entries = NULL;
    }
    free(symbol_table);
    symbol_table = NULL;
}

void SymTab_dump(struct SymTab* symbol_table, char* title)
{
    printf("\n");
    printf("%s:\n", title);
    for (int i = 0; i < symbol_table->table_size; i++) {
        struct SymTab_entry* entry = symbol_table->entries[i];
        if (entry == NULL) {
            continue;
        }
        printf("%4d:\t", i);
        printf("%s : %x", entry->key, entry->type);
        while (1) {
            if (entry->next == NULL) {
                break;
            }
            entry = entry->next;
            printf(",\n\t%s : %x", entry->key, entry->type);
        }
        printf("\n");
    }
    printf("\n");
}
