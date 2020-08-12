#pragma once

struct ConstTab {
    int size;
    long offset;
    void** entries;
    struct entry_list* start;
    struct entry_list* curr;
};

struct entry_list {
    void* entry;
    struct entry_list* next;
};

struct int_entry {
    long val;
    long offset;
    struct int_entry* next;
};

struct float_entry {
    double val;
    long offset;
    struct float_entry* next;
};

struct string_entry {
    char* val;
    long offset;
    struct string_entry* next;
};

struct ConstTab* create_ConstTab(int size);
struct int_entry* int_pair(long val, long* offset_ptr, struct entry_list** curr);
struct float_entry* float_pair(double val, long* offset_ptr, struct entry_list** curr);
struct string_entry* string_pair(char* val, long* offset_ptr, struct entry_list** curr);
struct int_entry* append_int(struct ConstTab* constant_table, long val);
struct float_entry* append_float(struct ConstTab* constant_table, double val);
struct string_entry* append_string(struct ConstTab* constant_table, char* val);
