#include <stdlib.h>
#include <pthread.h>

#include "length-check.h"

#ifndef FAT_UNUSED
#define FAT_UNUSED 65535
#define FAT_FILE_END 65534
#define FAT_BAD_CLUSTER 65533
#endif

#define CHECK_OK 0
#define CHECK_MISMATCH 1

struct check_farmer* create_check_farmer(FILE* p_file) {
	int root_directory_offset = 0, data_cluster_offset = 0,
			i;
	struct check_farmer* tmp = malloc(sizeof (struct check_farmer));

	// init boot record
	tmp->p_boot_record = malloc(sizeof (struct boot_record));
	fread(tmp->p_boot_record, sizeof (struct boot_record), 1, p_file);

	// init other structures
	tmp->fat_item = malloc(tmp->p_boot_record->cluster_count * sizeof (unsigned int));

	tmp->file_lock = malloc(sizeof (pthread_mutex_t));
	pthread_mutex_init(tmp->file_lock, NULL);
	tmp->file_count_lock = malloc(sizeof (pthread_mutex_t));
	pthread_mutex_init(tmp->file_count_lock, NULL);
	tmp->result_lock = malloc(sizeof (pthread_mutex_t));
	pthread_mutex_init(tmp->result_lock, NULL);

	// nacteni FAT
	for (i = 0; i < tmp->p_boot_record->fat_copies; i++) {
		fread(tmp->fat_item, sizeof (unsigned int), tmp->p_boot_record->cluster_count, p_file);
	}
	root_directory_offset = ftell(p_file);

	data_cluster_offset = root_directory_offset + tmp->p_boot_record->root_directory_max_entries_count * sizeof (struct root_directory);

	tmp->file_system = p_file;

	tmp->root_directory_offset = root_directory_offset;
	tmp->data_cluster_offset = data_cluster_offset;

	tmp->cur_file = 0;

	return tmp;
}

int delete_check_farmer(struct check_farmer* p_ch_f) {
	free(p_ch_f->fat_item);
	free(p_ch_f->p_boot_record);

	pthread_mutex_destroy(p_ch_f->file_lock);
	free(p_ch_f->file_lock);
	pthread_mutex_destroy(p_ch_f->file_count_lock);
	free(p_ch_f->file_count_lock);
	pthread_mutex_destroy(p_ch_f->result_lock);
	free(p_ch_f->result_lock);

	fclose(p_ch_f->file_system);

	free(p_ch_f);
	return 1;
}

struct check_worker* create_check_worker(struct check_farmer* p_ch_f, int w_id) {
	struct check_worker* tmp = malloc(sizeof (struct check_worker));

	tmp->p_root_directory = (struct root_directory *) malloc(sizeof (struct root_directory));
	tmp->p_cluster = malloc(sizeof (char) * p_ch_f->p_boot_record->cluster_size);
	tmp->worker_id = w_id;

	tmp->ch_f = p_ch_f;
	tmp->file_seq_num = 0;
	return tmp;
}

int delete_check_worker(struct check_worker* p_ch_w) {
	free(p_ch_w->p_root_directory);
	free(p_ch_w->p_cluster);
	free(p_ch_w);

	return 1;
}

int check_farmer_load_next_file(struct check_farmer* ch_f, struct root_directory* rd) {
	pthread_mutex_lock(ch_f->file_count_lock);
	int cur_file = ch_f->cur_file;
	if (cur_file >= ch_f->p_boot_record->root_directory_max_entries_count) {
		pthread_mutex_unlock(ch_f->file_count_lock);
		return 0;
	}
	ch_f->cur_file = cur_file + 1;
	pthread_mutex_unlock(ch_f->file_count_lock);

	pthread_mutex_lock(ch_f->file_lock);
	int file_offset = ch_f->root_directory_offset + cur_file * sizeof (struct root_directory);
	fseek(ch_f->file_system, file_offset, SEEK_SET);
	fread(rd, sizeof (struct root_directory), 1, ch_f->file_system);

	pthread_mutex_unlock(ch_f->file_lock);
	return 1;
}

int check_farmer_load_next_cluster(struct check_worker* p_ch_w, struct check_farmer* p_ch_f) {
	long cluster_offset = p_ch_f->data_cluster_offset + p_ch_w->next_cluster * sizeof (char) * p_ch_f->p_boot_record->cluster_size;
	pthread_mutex_lock(p_ch_f->file_lock);
	fseek(p_ch_f->file_system, cluster_offset, SEEK_SET);
	fread(p_ch_w->p_cluster, sizeof (char) * p_ch_f->p_boot_record->cluster_size, 1, p_ch_f->file_system);

	pthread_mutex_unlock(p_ch_f->file_lock);
	if (p_ch_w->next_cluster == FAT_BAD_CLUSTER || p_ch_w->next_cluster == FAT_FILE_END) {
		return 0;
	}
	if (p_ch_w->next_cluster > p_ch_f->p_boot_record->cluster_count) {
		return 0;
	}
	return p_ch_f->fat_item[p_ch_w->next_cluster];
}

void *check_worker_run(struct check_worker* p_ch_w) {
	unsigned int next_cl,
			total_length, current_clusters;
	unsigned int res[2];
	res[CHECK_OK] = res[CHECK_MISMATCH] = 0;
#ifdef DEBUG
	printf("Running worker %02d\n", p_ch_w->worker_id);
#endif
	while (check_farmer_load_next_file(p_ch_w->ch_f, p_ch_w->p_root_directory)) { // dokud jsou soubory ke kontrole
		p_ch_w->file_seq_num++;
		current_clusters = 0;


		next_cl = p_ch_w->p_root_directory->first_cluster;
		p_ch_w->next_cluster = check_farmer_load_next_cluster(p_ch_w, p_ch_w->ch_f);
		do {
			p_ch_w->next_cluster = next_cl;
			next_cl = check_farmer_load_next_cluster(p_ch_w, p_ch_w->ch_f);
			if (next_cl != FAT_FILE_END) {
				current_clusters++;
			} else {
				total_length = current_clusters * p_ch_w->ch_f->p_boot_record->cluster_size + strlen(p_ch_w->p_cluster);
			}
		} while (next_cl != FAT_BAD_CLUSTER && next_cl != FAT_FILE_END);
#ifdef DEBUG
		printf("(W%02d-F%03d): %16s => c: %d / e: %d \n",
				p_ch_w->worker_id, p_ch_w->file_seq_num,
				p_ch_w->p_root_directory->file_name,
				total_length, p_ch_w->p_root_directory->file_size);
#endif
		if (total_length == p_ch_w->p_root_directory->file_size) {
			res[CHECK_OK]++;
		} else {
			res[CHECK_MISMATCH]++;
		}
	} // iterace souboru
	pthread_mutex_lock(p_ch_w->ch_f->result_lock);
	p_ch_w->ch_f->results[CHECK_OK] += res[CHECK_OK];
	p_ch_w->ch_f->results[CHECK_MISMATCH] += res[CHECK_MISMATCH];
	pthread_mutex_unlock(p_ch_w->ch_f->result_lock);
}
