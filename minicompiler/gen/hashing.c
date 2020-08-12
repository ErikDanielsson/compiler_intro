#include <string.h>
unsigned int hash(const char* key, int table_size)
{
    // FVN (Fowler / Noll / Vo) hash function:
    unsigned int h = 0x811c9dc5;
    for (int i = 0; i < strlen(key); i++ ) {
        h = ( h ^ key[i] ) * 0x01000193;
    }
    return h % table_size;
}

unsigned int max_hash(const char* key)
{
    // FVN (Fowler / Noll / Vo) hash function:
    unsigned int h = 0x811c9dc5;
    for (int i = 0; i < strlen(key); i++ ) {
        h = ( h ^ key[i] ) * 0x01000193;
    }
    return h;
}

unsigned int ptr_hash(void* ptr, int table_size)
{
    long p = (long)ptr;
    unsigned int h = 0x811c9dc5;
    for (unsigned i = 0; i < 63; i++ )
        h = ( h ^ (char)(p >>= 1) ) * 0x01000193;

    return h % table_size;
}

unsigned int int_hash(long val, int table_size)
{
    unsigned int h = 0x811c9dc5;
    for (unsigned i = 0; i < 63; i++ )
        h = ( h ^ (char)(val >>= 1) ) * 0x01000193;

    return h % table_size;
}

unsigned int float_hash(double val, int table_size)
{
    long p = *(&val);
    unsigned int h = 0x811c9dc5;
    for (unsigned i = 0; i < 63; i++ )
        h = ( h ^ (char)(p >>= 1) ) * 0x01000193;

    return h % table_size;
}
