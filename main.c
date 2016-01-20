#include <stdio.h>
#include <stdlib.h>

#include "fat-operator.h"

#define OP_DEF_WRITE 1
#define OP_DEF_READ 2
#define OP_MY_CHECK 4
#define OP_MY_SHAKE 8

int operation;
int req_thread_count;

int process_parameters(int argc, char *argv[]) {
	int threads;
	if (argc > 0 && !strcmp("help", argv[0])) {
		return 1;
	}
	if (argc < 2) {
		printf("Not enough paramaters\n");
		return 2;
	}

	operation = -1;
	if (!strcmp(argv[0], "write")) {
		operation = OP_DEF_WRITE;
	}
	if (!strcmp(argv[0], "read")) {
		operation = OP_DEF_READ;
	}
	if (!strcmp(argv[0], "check")) {
		operation = OP_MY_CHECK;
	}
	if (!strcmp(argv[0], "shake")) {
		operation = OP_MY_SHAKE;
	}
	if (operation == -1) {
		printf("First argument representing operation (%s) was not recognised", argv[0]);
		return 3;
	}
	threads = atoi(argv[1]);
	if (threads < 1) {
		printf("Number of threads parameter '%s' is not a number.\n", argv[1]);
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
	if (process_parameters(argc - 1, argv + 1)) {
		print_help();
		return 1;
	}

	switch (operation) {
		case OP_DEF_WRITE:
			return main_write();
		case OP_DEF_READ:
			return main_read();
		case OP_MY_CHECK:
			return main_checkFileLength(req_thread_count);
		case OP_MY_SHAKE:
			return main_moveClustersToStart(req_thread_count);
	}
	//main_read();

	return 0;
}
