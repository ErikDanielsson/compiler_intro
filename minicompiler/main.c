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
#include "tree_climber.h"
#include "type_checker.h"
#include "table_generator.h"
#include "consts.h"
#include "intermediate_code.h"
#include "IC_table.h"
#include "live_and_use.h"
#include "code_generation.h"

int main(int argc, const char** argv)
{
    const char* table_file = "parsing/parsing_table.txt";
    filename = argv[1];
    char basename[strlen(filename)];
    char c;
    int i;
    for (i = 0; (c = filename[i]) != '.' && i < strlen(filename); i++)
        basename[i] = c;
    basename[i] = 0x00;
    file_desc = open(filename, O_RDONLY);
    init_lexer();
    #if VERBOSE
    KeywordTab_dump(keywords);
    #endif
    generate_parse_table(table_file);
    
    struct CompStmt* tree = lr_parser(1, basename);

    close(file_desc);
    if (!grammar_error) {
        init_type_checker();
        generate_IC(tree);

        live_and_use();
        generate_assembly(basename);
        
        destroy_CFG();
        destroy_Env_tree();
    }
    if (tree != NULL)
        free_CompStmt(tree);
    destroy_parse_table();
    KeywordTab_destroy(keywords);
}
