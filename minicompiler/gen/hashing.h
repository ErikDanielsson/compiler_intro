#pragma once
unsigned int hash(const char* key, int table_size);
unsigned int max_hash(const char* key);
unsigned int ptr_hash(void* ptr, int table_size);
unsigned int int_hash(long val, int table_size);
unsigned int float_hash(double val, int table_size);
