#ifndef CLUSTER_SHAKE_H
#define	CLUSTER_SHAKE

#include <pthread.h>
#include <semaphore.h>

#include "fat-operator.h"
// Struktura citajiciho farmare
struct shake_farmer {
	const int CLUSTER_CHUNK_SIZE;
	
	char* FS_path;
	FILE* file_system;
	
	int cluster_chunk_current;
	int cluster_chunks_not_empty;
	int cluster_chunk_last_size;
	int* cluster_chunk_read_beginings;
	int* cluster_chunk_read_ends;
	pthread_mutex_t* lock_cluster_chunk_counter;
	
	struct boot_record *p_boot_record;
	unsigned int *FAT;
	unsigned int *FAT_rev;
	struct root_directory *p_root_directory;
	
	char *cluster_content;
	sem_t* sem_cluster_access;

	long int offset_fat;
	long int offset_root_directory;
	long int offset_data_cluster;
};

// Struktura citace delky souboru
struct shake_worker {
	int worker_id;
	int nonfree_sleepers;
	
	struct shake_farmer *s_f;
	
	FILE* file_system_operator;
	
	int assigned_cluster_chunk;
	int search_chunk_start;
	int search_chunk_end;
	
	int search_index;
	unsigned int search_item;
	
	int chunk_put_offset;
	
	unsigned int hold_fat;
	unsigned char* hold_cluster;
};


int shake_analyze_fat(struct shake_farmer *p_s_f);

int write_shaken_headder(struct shake_farmer *p_s_f, char *file_name);

struct shake_farmer* create_shake_farmer(char* FS_path);

int delete_shake_farmer(struct shake_farmer* p_s_f);


struct shake_worker* create_shake_worker(struct shake_farmer* p_s_f, int w_id);


int delete_shake_worker(struct shake_worker* p_s_f);


int shake_farmer_load_next_file(struct shake_farmer* s_f, struct root_directory* rd);


int shake_farmer_load_next_cluster(struct shake_worker* p_s_w, struct shake_farmer* p_s_f);


void *shake_worker_run();

void shake_write_FAT_back(struct shake_farmer *p_s_f);

#endif	/* CLUSTER_SHAKE */
