#ifndef CLUSTER_SHAKE_H
#define	CLUSTER_SHAKE

#include <pthread.h>

#include "fat-operator.h"
// Struktura citajiciho farmare
struct shake_farmer {
	char* FS_path;
	FILE* file_system;
	const int CLUSTER_CHUNK_SIZE;
	
	int cluster_chunk_current;
	int cluster_chunks_total;
	int* cluster_chunk_read_beginings;
	pthread_mutex_t* lock_cluster_chunk;
	
	struct boot_record *p_boot_record;
	unsigned int *fat_item;
	struct root_directory *p_root_directory;
	char *cluster_content[];

	long int offset_fat;
	long int offset_root_directory;
	long int offset_data_cluster;
};

// Struktura citace delky souboru
struct shake_worker {
	int worker_id;
	
	struct shake_farmer *s_f;
	
	FILE* file_system_operator;
	
	int assigned_cluster_chunk;
	int cluster_num;
};


int shake_analyze_fat(shake_farmer *p_s_f);

struct shake_farmer* create_shake_farmer(char* FS_path);

int delete_shake_farmer(struct shake_farmer* p_s_f);


struct shake_worker* create_shake_worker(struct shake_farmer* p_s_f, int w_id);


int delete_shake_worker(struct shake_worker* p_s_f);


int shake_farmer_load_next_file(struct shake_farmer* s_f, struct root_directory* rd);


int shake_farmer_load_next_cluster(struct shake_worker* p_s_w, struct shake_farmer* p_s_f);


void *shake_worker_run(struct shake_worker* p_s_w);


#endif	/* CLUSTER_SHAKE */
