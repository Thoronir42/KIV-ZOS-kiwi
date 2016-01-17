#include <stdio.h>
#include <stdlib.h>

#include "fat-info.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char *argv[]) {
	for(int i = 1; i < argc; i ++){
		printf(argv[i]);
	}
	
	return 0;
}
