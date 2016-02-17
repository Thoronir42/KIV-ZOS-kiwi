#ifndef LENGTH_CHECK_H
#define	LENGTH_CHECK_H

#include <pthread.h>

#include "fat-operator.h"
// Struktura citajiciho farmare
struct check_farmer {
	struct boot_record *p_boot_record;
	FILE* file_system;
	pthread_mutex_t* file_lock;
	
	int cur_file;
	pthread_mutex_t* file_count_lock;
	
	int results[3];
	pthread_mutex_t* result_lock;

	long int root_directory_offset;
	long int data_cluster_offset;
	
	unsigned int *fat_item;
	struct root_directory *p_root_directory[];
};

// Struktura citace delky souboru
struct check_worker {
	struct root_directory *p_root_directory;
	struct check_farmer *ch_f;
	
	int worker_id;
	int file_seq_num;
	
	char* p_cluster;
	unsigned int next_cluster;
};

// vytvori strukturu citajiciho farmare a inicializuje jeho promenne
struct check_farmer* create_check_farmer(FILE* p_file);

// uvolni pamet pouzitou promennymi farmare a nasledne uvolni pamet po farmari
int delete_check_farmer(struct check_farmer* p_c_f);

// vytvori strukturu citajice a inicializuje jeho promenne
struct check_worker* create_check_worker(struct check_farmer* ch_f, int w_id);

// uvolni pamet pouzitou promennymi citace a nasledne uvolni pamet po citaci
int delete_check_worker(struct check_worker* p_c_f);


// od farmare nahraje informace o souboru do pameti citace
// resi se zde kriticka sekce pocitadla aktualniho souboru a kriticka sekce cteni ze souboru
// vraci 1, pokud byl dostupny soubor, nebo 0, pokud jsou jiz vsechny soubory zkontrolovany
int check_farmer_load_next_file(struct check_farmer* ch_f, struct root_directory* rd);

// citac si od farmare nacte dalsi cluster souboru
// resi se kriticka sekce cteni ze souboru
// vraci index dalsiho clusteru nebo 0, pokud je retez prerusen
int check_farmer_load_next_cluster(struct check_worker* p_ch_w, struct check_farmer* p_ch_f);

// citacova hlavni funkce : citac od farmare cte soubory, dokud farmar dava
// citac pak pro kazdy soubory od farmare cte dalsi clustery, dokud tvori retez
// a po dokonceni retezu vypise na obrazovku vypocitane informace
void *check_worker_run();


#endif	/* LENGTH_CHECK_H */
