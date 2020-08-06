#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type_table.h"
#include "hashing.h"

struct TypeTab* create_TypeTab(int table_size, struct TypeTab* parent)
{
    struct TypeTab* type_table = malloc(sizeof(struct TypeTab));
    type_table->table_size = table_size;
    type_table->entries = malloc(sizeof(struct TypeTab_entry*) * table_size);
    for (int i = 0; i < table_size; i++) {
        type_table->entries[i] = NULL;
    }
    return type_table;
}

struct TypeTab_entry* TypeTab_builtin_pair(char* key, int widening_priority)
{
    struct TypeTab_entry* entry = malloc(sizeof(struct TypeTab_entry));
    entry->key = malloc(sizeof(char)*strlen(key)+1);
    strcpy(entry->key, key);
    entry->type = TYPE_BUILTIN;
    entry->widening_priority = widening_priority;
    entry->next = NULL;
    return entry;
}

struct TypeTab_entry* TypeTab_struct_pair(char* key, void* struct_symbol)
{
    struct TypeTab_entry* entry = malloc(sizeof(struct TypeTab_entry));
    entry->key = malloc(sizeof(char)*strlen(key)+1);
    strcpy(entry->key, key);
    entry->type = TYPE_STRUCT;
    entry->struct_symbol = struct_symbol;
    entry->next = NULL;
    return entry;
}

void TypeTab_set_builtin(struct TypeTab* type_table, char* key, int widening_priority)
{
    unsigned int slot = hash(key, type_table->table_size);
    struct TypeTab_entry* entry = type_table->entries[slot];
    struct TypeTab_entry* prev;
    if (entry == NULL) {
        type_table->entries[slot] = TypeTab_builtin_pair(key, widening_priority);
        return;
    }
    do {
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = TypeTab_builtin_pair(key, widening_priority);
}

int get_widening_type(struct TypeTab* type_table, char* key)
{
    unsigned int hashv = hash(key, type_table->table_size);
    struct TypeTab_entry* entry = type_table->entries[hashv];
    while (entry != NULL) {
        if (entry->type == TYPE_BUILTIN && strcmp(entry->key, key) == 0)
            return entry->widening_priority;
        entry = entry->next;
    }
    return -1;
}

int TypeTab_check_and_set(struct TypeTab* type_table, char* key, void* struct_symbol)
{
    unsigned int slot = hash(key, type_table->table_size);
    struct TypeTab_entry* entry = type_table->entries[slot];
    struct TypeTab_entry* prev;
    if (entry == NULL) {
        type_table->entries[slot] = TypeTab_struct_pair(key, struct_symbol);
        return 0;
    }
    do {
        if (strcmp(entry->key, key) == 0)
            return 1;
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = TypeTab_struct_pair(key, struct_symbol);
    return 0;
}


int TypeTab_check_defined(struct TypeTab* type_table, char* key)
{
    unsigned int slot = hash(key, type_table->table_size);
    struct TypeTab_entry* entry = type_table->entries[slot];
    struct TypeTab_entry* prev;
    if (entry == NULL)
        return 0;
    do {
        if (strcmp(entry->key, key) == 0)
            return 1;
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    return 0;
}
