#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "lexer.h"
#include "keyword_table.h"
#include "parser.h"
#include "symbol_table.h"
#include "IC_gen.h"
#include "type_checker.h"
#include "table_generator.h"
#include "consts.h"


int main(int argc, const char** argv)
{
    const char* table_file = "parsing/parsing_table.txt";
    filename = argv[1];
    file_desc = open(filename, O_RDONLY);
    init_lexer();
    #if VERBOSE
    KeywordTab_dump(keywords);
    #endif
    generate_parse_table(table_file);
    init_type_checker();
    #if VERBOSE
    printf("parsing...\n");
    #endif
    struct CompStmt* tree = lr_parser(1);
    #if VERBOSE
    print_CompStmt((struct CompStmt*)(*record_ptr), 0, 1, 0);
    SymTab_dump(type_table, "Types", 0);
    #endif
    //if (return_found)
    //    return_error();
    close(file_desc);

    generate_IC(tree);
    #if VERBOSE
    print_Env_tree();
    #endif
    destroy_Env_tree();
    if (tree != NULL)
        free_CompStmt(tree);
    destroy_parse_table();
    KeywordTab_destroy(keywords);
}
