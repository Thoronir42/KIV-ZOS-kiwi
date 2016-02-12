#ifndef CLUSTER_SHAKE_H
#define	CLUSTER_SHAKE

#include <pthread.h>
#include <semaphore.h>

#include "fat-operator.h"
// Struktura farmare setreseni
struct shake_farmer {
	const int CLUSTER_CHUNK_SIZE;				// pevne dany pocet clusteru v chunku
	
	char* FS_path;								// cesta k "soubor. systemu"
	FILE* file_system;							// rozhrsni pro "soub. system"
	
	int cluster_chunk_current;					// pocitadlo zpracovanych chunku
	int cluster_chunks_not_empty;				// pocet neprazdnych chunku
	int cluster_chunk_last_size;				// pocet clusteru v poslednim chunku
	int* cluster_chunk_read_beginings;			// hranicni zacatky jednotlivych chunku
	int* cluster_chunk_read_ends;				// hranicni konce jednotlivych chunku
	pthread_mutex_t* lock_cluster_chunk_counter;// zamek pridelovace cluster chunku
	
	struct boot_record *p_boot_record;
	unsigned int *FAT;
	unsigned int *FAT_rev;						// prevracena FAT pro zachovaani retezcu
	struct root_directory *p_root_directory;
	int *rd_links;								// indexy na root directory zaznamy
	
	char *cluster_content;
	sem_t* sem_cluster_access;					// zamky jednotlivych clusteru

	long int offset_fat;
	long int offset_root_directory;
	long int offset_data_cluster;
};

// Struktura citace delky souboru
struct shake_worker {
	int worker_id;
	int nonfree_naps;			// kolikrat vlakno spalo protoze cilovy cluster nebyl volny
	
	struct shake_farmer *s_f;
	
	FILE* file_system_operator;
	
	int assigned_cluster_chunk;	// index zpracovavaneho chunku
	int search_chunk_start;		// zacatek zpracovavaneho chunku
	int search_chunk_end;		// konec zpracovavaneho chunku
	
	
	int search_index;			// prohledavany cluster
	unsigned int search_item;	// fat zaznam prohledavaneho clusteru
	
	int chunk_put_offset;		// zarovnani ukladaneho clusteru
	
	unsigned int hold_fat;
	unsigned char* hold_cluster;// docasne misto udrzujici obsah presouvaneho clusteru
};

// lists through root directory entries and marks clusters which are first in chain by a corresponding index
int shake_analyze_root_directory(struct shake_farmer* p_s_f);

// prepares information about FAT (reverse FAT, cluster chunks) for shaking
int shake_analyze_fat(struct shake_farmer *p_s_f);

// mocks secondary file for editing
int write_shaken_headder(struct shake_farmer *p_s_f, char *file_name);

// prepares a shake farmer stuct which will then hold informations needed for shaking
struct shake_farmer* create_shake_farmer(char* FS_path);

// frees memory allocated for shake farmer
int delete_shake_farmer(struct shake_farmer* p_s_f);

// creates shake worker stuct neccessary for thread which shakes clusters
struct shake_worker* create_shake_worker(struct shake_farmer* p_s_f, int w_id);

// frees memory allocated by shake worker
int delete_shake_worker(struct shake_worker* p_s_f);

// shake farmer assigns next cluster to shake worker if there still are any and returns 1, otherwise returns 0
int shake_next_cluster_chunk(struct shake_worker *p_s_w, struct shake_farmer* p_s_f);

// shake worker looks through assigned chunk cluster interval in FAT and assigns first valid cluster to proper variables, returns 1
// otherwise returns 0 which means that no valid cluster could be found within interval
int shake_worker_search_fat(struct shake_worker *p_s_w, struct shake_farmer *p_s_f);

// shake worker main function
void *shake_worker_run();

// once the shake is done, saves FAT and root directory entries into .FAT file
void shake_write_back(struct shake_farmer *p_s_f);

#endif	/* CLUSTER_SHAKE */
