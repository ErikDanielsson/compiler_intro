#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



int main(int argc, const char** argv)
{
	int input[] = {129, 43, 4, 43, 129, 43, 129, 4};
	const char* filename = argv[1];
	FILE* f_desc;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	int n_rules = 0;
	int n_states = 0;
	int n_terminals = 0;
	int n_nonterminals = 0;
	f_desc = fopen(filename, "r");
	if (f_desc == NULL)
		exit(EXIT_FAILURE);
	// Pass over file once to calculate lengths of arrays
	while ((read = getline(&line, &len, f_desc)) != -1) {
		if (line[0] == 'R' && line[1] == '\n')
			continue;
		if (line[0] == 'A' && line[1] == '\n')
			break;
		n_rules++;
	}
	read = getline(&line, &len, f_desc); // Read line with n_terminals
	char c  = line[0];
	for (int i = 0; isdigit(c); i++) {
		n_terminals = n_terminals*10 + c-0x30;
		c  = line[i+1];
	}
	n_terminals++;
	// Count number of states
	while ((read = getline(&line, &len, f_desc)) != -1) {

		if (line[0] == 'r' && line[1] == '\n')
			break;
		n_states++;
	}
	// Skip reduction rule lines
	while ((read = getline(&line, &len, f_desc)) != -1) {
		if (line[0] == 'G' && line[1] == '\n')
			break;
	}
	// Count nonterminals
	while ((read = getline(&line, &len, f_desc)) != -1) {
		if (line[0] == 'G' && line[1] == '\n')
			continue;
		n_nonterminals++;
	}
	fclose(f_desc);
	free(line);

	f_desc = fopen(filename, "r");
	if (f_desc == NULL)
		exit(EXIT_FAILURE);
	char** rules = malloc(sizeof(char*)*n_rules);
	line = NULL;
	len = 0;
	getline(&line, &len, f_desc); // Read line with 'R'
	for (int i = 0; i < n_rules; i++) {
		read = getline(&line, &len, f_desc);
		rules[i] = malloc(sizeof(char)*(read+1));
		strcpy(rules[i], line);
	}
	getline(&line, &len, f_desc); // Read line with n_terminals
	getline(&line, &len, f_desc); // Read line with 'A'
	int** action_table = malloc(sizeof(int*)*n_states);
	for (int i = 0; i < n_states; i++) {
		action_table[i] = malloc(sizeof(int)*n_terminals);
		// Map erroneous entries to values above maximum state number
		for (int j = 0; j < n_terminals; j++)
			action_table[i][j] = n_states;
		getline(&line, &len, f_desc);
		for (int j = 0; line[j] != '\n';) {
			int index = 0;
			int value = 0;
			for (;isdigit(line[j]); j++) {
				index = index*10 + line[j]-0x30;
			}
			j++;
			char b = (line[j] == '-');
			j += b;
			for (;isdigit(line[j]); j++)
				value = value*10 + line[j]-0x30;
			action_table[i][index] = value - b*2*value;
			j++;
		}
	}
	for (int i = 0; i < n_states; i++) {
		printf("%d: ", i);
		for (int j = 0; j < n_terminals; j++) {
			printf("%d ", action_table[i][j]);
		}
		printf("\n");
	}
	getline(&line, &len, f_desc); // Read line with 'r'
	getline(&line, &len, f_desc);
	int* reduction_rules = malloc(sizeof(int)*n_rules);
	int index = 0;
	for (int i = 0; line[i] != '\n'; index++) {
		int tmp = 0;
		while (line[i] != ',') {
			tmp = tmp*10 + line[i]-0x30;
			i++;
		}
		i++;
		reduction_rules[index] = tmp;
	}
	getline(&line, &len, f_desc);
	int* n_pop_states = malloc(sizeof(int)*n_rules);
	index = 0;
	for (int i = 0; line[i] != '\n'; index++) {
		int tmp = 0;
		while (line[i] != ',') {
			tmp = tmp*10 + line[i]-0x30;
			i++;
		}
		i++;
		n_pop_states[index] = tmp;
	}

	getline(&line, &len, f_desc); // Read line with G
	int** goto_table = malloc(sizeof(int*)*n_nonterminals);
	for (int i = 0; i < n_nonterminals; i++) {
		read = getline(&line, &len, f_desc);
		if (read == 1) {
			goto_table[i] = NULL;
			continue;
		}
		goto_table[i] = malloc(sizeof(int)*n_states);
		for (int q = 0; q < n_states; q++)
			goto_table[i][q] = -1;
		int index = 0;
		for (int j = 0; line[j] != '\n';) {
			int index = 0;
			int value = 0;
			for (;isdigit(line[j]); j++) {
				index = index*10 + line[j]-0x30;
			}
			j++;
			for (;isdigit(line[j]); j++)
				value = value*10 + line[j]-0x30;
			goto_table[i][index] = value;
			j++;
		}
	}

	fclose(f_desc);
	lr_parser(input, rules, reduction_rules, n_pop_states,
		  action_table, n_states, goto_table);
	free(line);
	return 0;
}
