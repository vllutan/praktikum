#include <stdio.h>
#include <stdlib.h>

int main(){
	char* s = malloc(10);
	
	free(s);
	return 0;
}
