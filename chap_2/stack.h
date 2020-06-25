#pragma once
include "symbol_table.h"

struct SymTabStack {
	int top;
	struct SymTab **arr;
};

void SymTabStack_push(struct SymTabStack* stack, struct SymTab* symtab);
struct SymTab* SymTabStack_pop(struct SymTabStack* stack);
