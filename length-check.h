#ifndef LENGTH_CHECK_H
#define	LENGTH_CHECK_H

#include "fat-operator.h"

struct check_farmer {
	struct boot_record *p_boot_record;
	FILE* file_system;
	
	int cur_file;
	
	long int root_directory_offset;
	long int data_cluster_offset;

};

struct check_worker {
	struct root_directory *p_root_directory;
	struct check_farmer *ch_f;
	int n;
};

struct check_farmer* create_check_farmer(FILE* p_file, struct boot_record *p_boot_record);

int delete_check_farmer(struct check_farmer* p_c_f);


struct check_worker* create_check_worker(struct check_farmer* ch_f);

int delete_check_worker(struct check_worker* p_c_f);


struct root_directory* check_farmer_load_next_file(struct check_farmer* ch_f, struct root_directory* rd) ;

struct root_directory* check_farmer_next_file(struct check_farmer*, struct check_worker*);


#endif	/* LENGTH_CHECK_H */
