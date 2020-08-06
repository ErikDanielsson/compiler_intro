extern char** rules;
extern int* reduction_rules;
extern int* n_pop_states;
extern int** action_table;
extern int n_states;
extern int** goto_table;

void generate_parse_table();
void destroy_parse_table();
