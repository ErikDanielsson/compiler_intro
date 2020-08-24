#pragma once

enum syscall_const {
    SYSEXIT
};

void generate_assembly(const char* basename);
void store(struct SymTab_entry* var, int reg);

void load_int_to_reg(struct SymTab_entry* entry, unsigned int loged_width, unsigned int reg_n)
{
    write("mov %s, ", register_names[loged_width][new_reg])
    switch (loged_width) {
        case 0:
        case 1:
        default:
            fprintf(stderr, "Internal error:8 and 16 bit ints are currently not supported\n");
            exit(-1);
        case 2:
            write("%d", (int)rval_entry->val);
            break;
        case 3:
            write("%ld", (long)rval_entry->val)
            break;
    }
    write("\n");
}

void load_int_to_mem(long entry_val, unsigned int loged_width, char* mem_loc_str)
{
    write("mov %s %s", size_spec[loged_width], mem_loc_str);
    switch (loged_width) {
        case 0:
        case 1:
        default:
            fprintf(stderr, "Internal error:8 and 16 bit ints are currently not supported\n");
            exit(-1);
        case 2:
            write("%d", (int)entry_val);
            break;
        case 3:
            write("%ld", (long)entry_val)
            break;
    }
    write("\n");
}

static inline void get_memstr(char** memstr_loc, unsigned int isstatic, struct SymTab_entry* entry)
{
    if (isstatic)
        sprintf(*memstr_loc, "[%s%u]", entry->key, entry->counter_value);
    else
        sprintf(*memstr_loc, "[rbp+%u]", entry->offset);
}
