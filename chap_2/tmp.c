#include <stdio.h>

int main() {
	FILE *fp = fopen("symbols.txt", "r");
	int c;
	int n = 0;
	if (fp == NULL) {
		return -1;
	} do {
      c = fgetc(fp);
      if( feof(fp) ) {
         break ;
      }
      printf("%c", c);
   } while(1);
	fclose(fp);
	return 0;
}
