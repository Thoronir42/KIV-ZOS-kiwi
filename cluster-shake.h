#ifndef CLUSTER_SHAKE_H
#define	CLUSTER_SHAKE

#include <pthread.h>

#include "fat-operator.h"
// Struktura citajiciho farmare
struct shake_farmer {
	struct boot_record *p_boot_record;
	FILE* file_system;
	pthread_mutex_t* file_lock;
	
	int cur_file;
	pthread_mutex_t* file_count_lock;

	long int root_directory_offset;
	long int data_cluster_offset;
	
	unsigned int *fat_item;
	struct root_directory *p_root_directory[];
};

// Struktura citace delky souboru
struct shake_worker {
	struct root_directory *p_root_directory;
	struct shake_farmer *s_f;
	
	int worker_id;
	int file_seq_num;
	
	char* p_cluster;
	unsigned int next_cluster;
};


struct shake_farmer* create_shake_farmer(FILE* p_file);


int delete_shake_farmer(struct shake_farmer* p_s_f);


struct shake_worker* create_shake_worker(struct shake_farmer* p_s_f, int w_id);


int delete_shake_worker(struct shake_worker* p_s_f);


int shake_farmer_load_next_file(struct shake_farmer* s_f, struct root_directory* rd);


int shake_farmer_load_next_cluster(struct shake_worker* p_s_w, struct shake_farmer* p_s_f);


void *shake_worker_run(struct shake_worker* p_s_w);


#endif	/* CLUSTER_SHAKE */
