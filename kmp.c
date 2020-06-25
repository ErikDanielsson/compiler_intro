#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int kmp(char* string, char* keyword)
{
	int s_len = strlen(string);
	int fail[s_len];
	int t = 0;
	fail[0] = 0;
	for (int s=1; s<s_len; s++) {
		while (t > 0 && string[s] != string[t])
			t = fail[t-1];
		if (string[s] == string[t]) {
			t++;
			fail[s] = t;
		} else {
			fail[s] = t;
		}
	}
	//printf("%s\n", string);
	//for (int i = 0; i < s_len; i++)
	//	printf("%d", fail[i]);
	//printf("\n");
	t = 0;
	int k_len = strlen(keyword);
	for (int i = 0; i < s_len; i++) {
		while (t > 0 && string[i] != keyword[t])
			t = fail[t-1];
		if (string[i] == keyword[t])
			t++;
		if (t == k_len)
		 	return i;
	}
	return -1;
}

int fib(int n) {
	int a = 0;
	int b = 1;
	int c = b;
	for (int i = 0; i < n; i++) {
		c = a+b;
		a = b;
		b = c;
	}
	return b;
}

char* fibstring(int n) {
	int s = fib(n);
	char a[s];
	char b[s];
	char* c = malloc(sizeof(b));
	a[0] = 0x61;
	a[1] = 0x00;
	b[0] = 0x62;
	b[1] = 0x00;
	for (int i = 0; i<n; i++) {
		strcat(c, a);
		strcpy(a, b);
		strcpy(b, c);
		strrev(c);
	}
	return c;
}

int main(int argc, char** argv) {
	char* string = fibstring(atoi(argv[1]));
	char* keyword = fibstring(atoi(argv[1])-1);
	int index = kmp(string, keyword);

	if (index == -1) {
		printf("No match!\n");
		return 0;
	}

	int s_len = strlen(string);
	int k_len = strlen(keyword);
	index = index - k_len +1;

	for (int i = 0; i < s_len; i++) {
		if (i == index) {
			printf("\033[1;31m");
			for (int j; j < k_len; j++) {
				printf("%c", string[i]);
				i++;
			}
			printf("\033[0m");
			i--;
		} else {
			printf("%c", string[i]);
		}
	}
	printf("\n");
	return 0;
}
