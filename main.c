#include <stdio.h>
#include <stdlib.h>

#include "fat-operator.h"

int req_thread_count;

int process_parameters(int argc, char *argv[]) {
	int threads;
	if (argc < 1) {
		printf("Not enough paramaters\n");
		return 2;
	}
	if (!strcmp("help", argv[0])) {
		return 1;
	}
	threads = atoi(argv[0]);
	if (threads < 1) {
		printf("Number of threads parameter '%s' is not a number.\n", argv[0]);
		return 3;
	}
	
	req_thread_count = threads;

	return 0;
}

int print_help() {
	printf("Usage: \n");
	printf("soubor [threads]\n");
	printf("\t[threads]: amount of threads to be used for current run\n");
	return 0;
}

int main(int argc, char *argv[]) {
	if (process_parameters(argc - 1, argv + 1)) {
			print_help();
			return 1;
	}
	printf("Pocet vlaken %d", req_thread_count);
	//main_read();

	return 0;
}
