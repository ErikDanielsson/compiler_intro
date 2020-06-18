/*
GRAMMAR
S -> S (S) S | epsilon


The dragon book states that a left recursive grammar:

A -> Aa|b

can be transformed into the following right recursive grammar:

A -> bR
R -> aR|epsilon

If this mask is applied to the grammar defined above,
one finds that A, a and b takes the following values:

A := S
a := (S) S
b := epsilon

=> A -> bR <=> S -> R && R -> (S) S R | epiloson

which means that the right recursive grammar is:

S -> R
R -> (S) S R | epsilon

and since the first production states that S and R are equivalent,
the grammar can be further simplified into:

S -> (S) S S | epsilon

Now follows an implementation:
*/

#include <stdio.h>


char token;

char get_next_token() {
	return token = getchar();
}



void match(char t) {
	if (token == t) {
		return;
	}
	else {
		fprintf(stderr, "Syntax error %c", token);
	}
}

void S();
void R();

int main() {
	while (1) {
		printf(">>> ");
		fflush(stdout);
		get_next_token();
		R();
	}

}

void R() {
	if (token != '(')
		return;
	match('(');
	get_next_token();
	R();
	match(')');
	get_next_token();

	R();
	R();

}
