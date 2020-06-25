#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#define STACKHEIGHT 64 // i.e maximum nesting level

struct SymTabStack* SymTabStack_create()
{
	struct SymTabStack* stack = malloc(sizeof(struct SymTabStack));
	stack->top = 0;
	stack->arr = malloc(sizeof(struct Symtab*) * STACKHEIGHT);
	for (int i = 0; i < STACKHEIGHT; i++)
		stack->arr[i] = NULL;
	return stack;
}

void SymTabStack_destroy(struct SymTabStack* stack)
{
	// Requires the stack is fully popped
	free(stack->arr);
	free(stack);
}

void SymTabStack_push(struct SymTabStack* stack, struct SymTab* symtab)
{
	int i = stack->top;
	if (i < STACKHEIGHT) {
		stack->top++;
		stack->arr[i] = symtab;
	} else {
		fprintf(stderr, "error: Too deep nesting");
	}
}

struct SymTab* SymTabStack_pop(struct SymTabStack* stack);
{
	int i = stack->top;
	struct Symtab* tmp = stack->arr[i];
	stack->arr[i] = NULL;
	stack->top--;
	return tmp;
}
