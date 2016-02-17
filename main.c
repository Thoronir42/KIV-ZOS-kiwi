#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#include "fat-operator.h"

#define OP_DEF_WRITE 1
#define OP_DEF_READ 2
#define OP_MY_CHECK 4
#define OP_MY_SHAKE 8

#define DEBUG 1

int operation;
int req_thread_count;
char read_src[30];

int process_parameters(int argc, char *argv[]) {
	operation = -1;
	int threads;
	if (argc < 1) {
		printf("Not enough paramaters\n");
		return 2;

	} else if (!strcmp("write", argv[0])) {
		operation = OP_DEF_WRITE;
		return 0;

	} else if (!strcmp("read", argv[0])) {
		if (argc == 2) {
			strcpy(read_src, argv[1]);
		} else {
			strcpy(read_src, "output.fat");
		}
		operation = OP_DEF_READ;
		return 0;

	} else if (!strcmp("help", argv[0])) {
		return 1;
	}

	if (argc < 2) {
		printf("Not enough paramaters for %s action\n", argv[0]);
		return 2;

	} else if (!strcmp(argv[0], "check")) {
		operation = OP_MY_CHECK;
		
	} else if (!strcmp(argv[0], "shake")) {
		operation = OP_MY_SHAKE;
		
	}
	if (operation == -1) {
		printf("First argument representing operation (%s) was not recognised", argv[0]);
		return 3;
	}
	threads = atoi(argv[1]);
	if (threads < 1 || threads > 64) {
		printf("Number of threads parameter '%s' is not a valid number. Please choose a number within <1, 64>\n", argv[1]);
		return 4;
	}

	req_thread_count = threads;

	return 0;
}

int print_help() {
	printf("Usage: soubor [operation] [threads]\n");
	printf("\t[operation]: what operation to use (help|write|read|check|shake)\n");
	printf("\t\t help  : shows this message\n");
	printf("\t\t write : writes a sample output file with testing FAT data\n");
	printf("\t\t read  : scan over output data file and print FAT info\n");
	printf("\t\t check : check if all files in output FAT file are correct length\n");
	printf("\t\t shake : move all data clusters to begining of FAT space\n");

	printf("\t[threads]: amount of threads to be used for current run\n");
	return 0;
}

int main(int argc, char *argv[]) {
	int op_result;
	if (process_parameters(argc - 1, argv + 1)) {
		print_help();
		return 1;
	}

	clock_t t_start, t_end;
	float total_time;

	t_start = clock();
#ifdef DEBUG
	printf("Starting operation %s\n", argv[1]);
#endif
	switch (operation) {
		case OP_DEF_WRITE:
			op_result = main_write();
			break;
		case OP_DEF_READ:
			op_result = main_read(read_src);
			break;
		case OP_MY_CHECK:
			op_result = main_checkFileLength(req_thread_count);
			break;
		case OP_MY_SHAKE:
			op_result = main_moveClustersToStart(req_thread_count);
			break;
	}

	t_end = clock();
	total_time = ((float) (t_end - t_start) / CLOCKS_PER_SEC) * 1000;
#ifdef DEBUG
	printf("Job %s with %d threads took %.02f ms.\n", argv[1], req_thread_count, total_time);
#else
	printf("%.02f ms", total_time);
#endif

	return op_result;
}
