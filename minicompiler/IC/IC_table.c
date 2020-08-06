#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IC_table.h"
#include "hashing.h"

struct IC_table* create_IC_table(int table_size)
{
    struct IC_table* code_table = malloc(sizeof(struct IC_table));
    code_table->size = table_size;
    code_table->entries = malloc(sizeof(struct IC_entry*)*table_size);
    for (int i = 0; i < table_size; i++)
        code_table->entries[i] = NULL;
    return code_table;
}

struct IC_entry* IC_table_pair(char* key)
{
    struct IC_entry* entry = malloc(sizeof(struct IC_entry));
    entry->key = malloc(sizeof(char)*(strlen(key)+1));
    strcpy(entry->key, key);
    entry->instruction_list = NULL;
    entry->next = NULL;
    return entry;
}

void insertion_error(char* key)
{
    fprintf(stderr, "Internal error:There is already a function with the name '%s',\n somethings up with semantic analysis\n", key);
    exit(-1);
}

void IC_table_create_entry(struct IC_table* code_table, char* key)
{
    unsigned int hashv = hash(key, code_table->size);
    struct IC_entry* entry = code_table->entries[hashv];
    struct IC_entry* prev;
    if (entry == NULL) {
        code_table->entries[hashv] = IC_table_pair(key);
        return;
    }
    do {
        if (strcmp(entry->key, key) == 0)
            insertion_error(key);
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = IC_table_pair(key);
}

void lookup_error(char* key)
{
    fprintf(stderr, "Internal error:There is no function with the name '%s' in code table\n", key);
    exit(-1);
}

struct IC_entry* IC_table_get_entry(struct IC_table* code_table, char* key)
{
    unsigned int hashv = hash(key, code_table->size);
    struct IC_entry* entry = code_table->entries[hashv];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0)
            return entry;
        entry = entry->next;
    }
    lookup_error(key);
    return NULL;
}
