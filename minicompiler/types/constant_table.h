#pragma once

struct ConstTab {
    int size;
    void** entries;
};

struct int_entry {
    long val;
    struct int_entry* next;
};

struct float_entry {
    double val;
    struct float_entry* next;
};

struct string_entry {
    char* val;
    struct string_entry* next;
};

struct ConstTab* create_ConstTab(int size);
struct int_entry* int_pair(long val);
struct float_entry* float_pair(double val);
struct string_entry* string_pair(char* val);
struct int_entry* append_int(struct ConstTab* constant_table, long val);
struct float_entry* append_float(struct ConstTab* constant_table, double val);
struct string_entry* append_string(struct ConstTab* constant_table, char* val);
