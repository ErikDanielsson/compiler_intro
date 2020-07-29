#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "hashing.h"

struct SymTab* create_SymTab(int table_size)
{
    struct SymTab* symbol_table = malloc(sizeof(struct SymTab));
    symbol_table->table_size  = table_size;
    symbol_table->entries = malloc(sizeof(struct SymTab_entry*) * table_size);

    for (int i = 0; i < table_size; i++) {
        symbol_table->entries[i] = NULL;
    }

    return symbol_table;
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
    return entry;
}

void SymTab_set(struct SymTab* symbol_table, char* key,
                    enum SymbolType type, void* symbol)
{
    unsigned int slot = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry = symbol_table->entries[slot];
    struct SymTab_entry* prev;
    if (entry == NULL) {
        symbol_table->entries[slot] = SymTab_pair(key, type, symbol);
        return;
    }
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            entry->type = type;
            return;
        }
        prev = entry;
        entry = prev->next;
    }
    prev->next = SymTab_pair(key, type, symbol);
}

enum SymbolType SymTab_get(struct SymTab* symbol_table, char* key,
                            void** symbol_out)
{
    unsigned int slot = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry = symbol_table->entries[slot];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->type;
            *symbol_out = entry->symbol;
        }
        entry = entry->next;
    }
    return -1;
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

void SymTab_dump(struct SymTab* symbol_table)
{
    printf("\n");
    printf("Symbols:\n");
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
