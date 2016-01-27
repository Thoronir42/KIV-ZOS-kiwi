#include <stdlib.h>
#include <pthread.h>

#include "cluster-shake.h"

#ifndef FAT_UNUSED
#define FAT_UNUSED 65535
#define FAT_FILE_END 65534
#define FAT_BAD_CLUSTER 65533
#endif

struct shake_farmer* create_shake_farmer(FILE* p_file) {
	int root_directory_offset = 0, data_cluster_offset = 0;
	struct shake_farmer* tmp = malloc(sizeof (struct shake_farmer));

	// init boot record
	tmp->p_boot_record = malloc(sizeof (struct boot_record));
	fread(tmp->p_boot_record, sizeof (struct boot_record), 1, p_file);

	// init other structures
	tmp->fat_item = malloc(tmp->p_boot_record->cluster_count * tmp->p_boot_record->fat_copies * sizeof (unsigned int));

	tmp->file_lock = malloc(sizeof (pthread_mutex_t));
	tmp->file_count_lock = malloc(sizeof (pthread_mutex_t));
	pthread_mutex_init(tmp->file_lock, NULL);
	pthread_mutex_init(tmp->file_count_lock, NULL);

	// nacteni FAT
	fread(tmp->fat_item, sizeof (unsigned int), tmp->p_boot_record->cluster_count * tmp->p_boot_record->fat_copies, p_file);
	root_directory_offset = ftell(p_file);

	data_cluster_offset = root_directory_offset + tmp->p_boot_record->root_directory_max_entries_count * sizeof (struct root_directory);

	tmp->file_system = p_file;

	tmp->root_directory_offset = root_directory_offset;
	tmp->data_cluster_offset = data_cluster_offset;

	tmp->cur_file = 0;

	return tmp;
}

int delete_shake_farmer(struct shake_farmer* p_s_f) {
	free(p_s_f->fat_item);
	free(p_s_f->p_boot_record);
	pthread_mutex_destroy(p_s_f->file_lock);
	pthread_mutex_destroy(p_s_f->file_count_lock);
	free(p_s_f->file_lock);
	free(p_s_f->file_count_lock);
	fclose(p_s_f->file_system);

	free(p_s_f);
	return 1;
}

struct shake_worker* create_shake_worker(struct shake_farmer* p_s_f, int w_id){
	struct shake_worker* tmp = malloc(sizeof (struct shake_worker));

	tmp->p_root_directory = (struct root_directory *) malloc(sizeof (struct root_directory));
	tmp->p_cluster = malloc(sizeof (char) * p_s_f->p_boot_record->cluster_size);
	tmp->worker_id = w_id;

	tmp->s_f = p_s_f;
	tmp->file_seq_num = 0;
	return tmp;
}

int delete_shake_worker(struct shake_worker* p_s_w){
	free(p_s_w->p_root_directory);
	free(p_s_w->p_cluster);
	free(p_s_w);

	return 1;
}

int shake_farmer_load_next_file(struct shake_farmer* s_f, struct root_directory* rd){
	pthread_mutex_lock(s_f->file_count_lock);
	int cur_file = s_f->cur_file;
	if (cur_file >= s_f->p_boot_record->root_directory_max_entries_count) {
		pthread_mutex_unlock(s_f->file_count_lock);
		return 0;
	}
	s_f->cur_file = cur_file + 1;
	pthread_mutex_unlock(s_f->file_count_lock);

	pthread_mutex_lock(s_f->file_lock);
	int file_offset = s_f->root_directory_offset + cur_file * sizeof (struct root_directory);
	fseek(s_f->file_system, file_offset, SEEK_SET);
	fread(rd, sizeof (struct root_directory), 1, s_f->file_system);

	pthread_mutex_unlock(s_f->file_lock);
	return 1;
}

int shake_farmer_load_next_cluster(struct shake_worker* p_s_w, struct shake_farmer* p_s_f){
	long cluster_offset = p_s_f->data_cluster_offset + p_s_w->next_cluster * sizeof (char) * p_s_f->p_boot_record->cluster_size;
	pthread_mutex_lock(p_s_f->file_lock);
	fseek(p_s_f->file_system, cluster_offset, SEEK_SET);
	fread(p_s_w->p_cluster, sizeof (char) * p_s_f->p_boot_record->cluster_size, 1, p_s_f->file_system);

	pthread_mutex_unlock(p_s_f->file_lock);
	if (p_s_w->next_cluster == FAT_BAD_CLUSTER || p_s_w->next_cluster == FAT_FILE_END) {
		return 0;
	}
	if (p_s_w->next_cluster > p_s_f->p_boot_record->cluster_count) {
		return 0;
	}
	return p_s_f->fat_item[p_s_w->next_cluster];
}

void *shake_worker_run(struct shake_worker* p_s_w){ 
	unsigned int next_cl,
			total_length,
			cluster_length;
	//printf("Running worker %02d\n", p_ch_w->worker_id);
	while (shake_farmer_load_next_file(p_s_w->s_f, p_s_w->p_root_directory)) {
		p_s_w->file_seq_num++;
		total_length = 0;


		next_cl = p_s_w->p_root_directory->first_cluster;
		p_s_w->next_cluster = shake_farmer_load_next_cluster(p_s_w, p_s_w->s_f);
		do {
			p_s_w->next_cluster = next_cl;
			next_cl = shake_farmer_load_next_cluster(p_s_w, p_s_w->s_f);
			total_length += (cluster_length = strlen(p_s_w->p_cluster));
			//printf("cl %04d\t%d\t %s\n", p_ch_w->next_cluster, cluster_length, p_ch_w->p_cluster);

		} while (next_cl != FAT_BAD_CLUSTER && next_cl != FAT_FILE_END);
		printf("(W%02d-F%03d): %16s => c: %d / e: %d \n",
				p_s_w->worker_id, p_s_w->file_seq_num,
				p_s_w->p_root_directory->file_name,
				total_length, p_s_w->p_root_directory->file_size);
	}
	//printf("Worker %02d don.\n", p_ch_w->worker_id);
}
