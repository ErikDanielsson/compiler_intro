#include <stdio.h>

void find_ptr_error(void* ptr) {
	if (ptr == NULL) {
		fprintf(stderr, "FATAL ERROR: Unable to find heap space");
	}
}
