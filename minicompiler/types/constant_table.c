#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constant_table.h"
#include "hashing.h"

struct ConstTab* create_ConstTab(int size)
{
    struct ConstTab* constant_table = malloc(sizeof(struct ConstTab));
    constant_table->size = size;
    constant_table->offset = 0;
    constant_table->entries = malloc(sizeof(void*)*size);
    constant_table->start = malloc(sizeof(struct entry_list));
    constant_table->curr = constant_table->start;
    for (int i = 0; i < size; i++)
        constant_table->entries[i] = NULL;
    return constant_table;
}

struct int_entry* int_pair(long val, long* offset_ptr, struct entry_list** curr)
{
    struct int_entry* entry = malloc(sizeof(struct int_entry));
    entry->val = val;
    entry->offset = *offset_ptr;
    entry->locs = 1;
    *offset_ptr += 8;
    entry->next = NULL;
    (*curr)->entry = entry;
    (*curr)->next = malloc(sizeof(struct entry_list));
    *curr = (*curr)->next;
    (*curr)->next = NULL;
    return entry;
}

struct float_entry* float_pair(double val, long* offset_ptr, struct entry_list** curr)
{
    struct float_entry* entry = malloc(sizeof(struct float_entry));
    entry->val = val;
    entry->offset = *offset_ptr;
    entry->locs = 1;
    *offset_ptr += 8;
    entry->next = NULL;
    (*curr)->entry = entry;
    (*curr)->next = malloc(sizeof(struct entry_list));
    *curr = (*curr)->next;
    (*curr)->next = NULL;
    return entry;
}

struct string_entry* string_pair(char* val, long* offset_ptr, struct entry_list** curr)
{
    struct string_entry* entry = malloc(sizeof(struct string_entry));
    entry->val = val;
    entry->offset = *offset_ptr;
    entry->locs = 1;
    *offset_ptr += 8;
    entry->next = NULL;
    (*curr)->entry = entry;
    (*curr)->next = malloc(sizeof(struct entry_list));
    *curr = (*curr)->next;
    (*curr)->next = NULL;
    return entry;
}

struct int_entry* append_int(struct ConstTab* constant_table, long val)
{
    unsigned int hashv = int_hash(val, constant_table->size);
    struct int_entry* entry = constant_table->entries[hashv];

    struct int_entry* prev;

    if (entry == NULL) {
        constant_table->entries[hashv] = int_pair(val,
                                                &(constant_table->offset),
                                                &(constant_table->curr));
        return constant_table->entries[hashv];
    }
    do {
        if (entry->val == val)
            return entry;
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = int_pair(val, &(constant_table->offset), &(constant_table->curr));
    return prev->next;
}

struct float_entry* append_float(struct ConstTab* constant_table, double val)
{
    unsigned int hashv = float_hash(val, constant_table->size);
    struct float_entry* entry = constant_table->entries[hashv];
    struct float_entry* prev;
    if (entry == NULL) {
        constant_table->entries[hashv] = float_pair(val,
                                                    &(constant_table->offset),
                                                    &(constant_table->curr));
        return constant_table->entries[hashv];
    }
    do {
        if (entry->val == val)
            return entry;
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = float_pair(val, &(constant_table->offset), &(constant_table->curr));
    return prev->next;
}

struct string_entry* append_string(struct ConstTab* constant_table, char* val)
{
    unsigned int hashv = hash(val, constant_table->size);
    struct string_entry* entry = constant_table->entries[hashv];
    struct string_entry* prev;
    if (entry == NULL) {
        constant_table->entries[hashv] = string_pair(val,
                                                    &(constant_table->offset),
                                                    &(constant_table->curr));
        return constant_table->entries[hashv];
    }
    do {
        if (strcmp(entry->val, val) == 0)
            return entry;
        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = string_pair(val, &(constant_table->offset), &(constant_table->curr));
    return prev->next;
}
