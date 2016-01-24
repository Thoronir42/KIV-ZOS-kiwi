#ifndef LENGTH_CHECK_H
#define	LENGTH_CHECK_H

#include "fat-operator.h"

struct check_farmer {
	struct boot_record *p_boot_record;
	FILE* file_system;
	
	int cur_file;

	long int root_directory_offset;
	long int data_cluster_offset;
	
	unsigned int *fat_item;
	struct root_directory *p_root_directory[];
};

struct check_worker {
	struct root_directory *p_root_directory;
	struct check_farmer *ch_f;
	int file_seq_num;
	
	char* p_cluster;
	unsigned int next_cluster;
};

struct check_farmer* create_check_farmer(FILE* p_file, struct boot_record *p_boot_record);

int delete_check_farmer(struct check_farmer* p_c_f);


struct check_worker* create_check_worker(struct check_farmer* ch_f);

int delete_check_worker(struct check_worker* p_c_f);


int check_farmer_load_next_file(struct check_farmer* ch_f, struct root_directory* rd);

int check_worker_run();


#endif	/* LENGTH_CHECK_H */
