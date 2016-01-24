#include <stdlib.h>

#include "length-check.h"

#ifndef FAT_UNUSED
#define FAT_UNUSED 65535
#define FAT_FILE_END 65534
#define FAT_BAD_CLUSTER 65533
#endif

struct check_farmer* create_check_farmer(FILE* p_file, struct boot_record *p_boot_record) {
	printf("Creating check_farmer\n");
	struct check_farmer* tmp = malloc(sizeof (struct check_farmer));
	tmp->fat_item = malloc(p_boot_record->cluster_count * p_boot_record->fat_copies * sizeof (unsigned int));

	int root_directory_offset = 0, data_cluster_offset = 0;

	// nacteni FAT
	fread(tmp->fat_item, sizeof (unsigned int), p_boot_record->cluster_count * p_boot_record->fat_copies, p_file);
	root_directory_offset = ftell(p_file);

	data_cluster_offset = root_directory_offset + p_boot_record->root_directory_max_entries_count * sizeof (struct root_directory);

	tmp->p_boot_record = p_boot_record;
	tmp->file_system = p_file;

	tmp->root_directory_offset = root_directory_offset;
	tmp->data_cluster_offset = data_cluster_offset;

	tmp->cur_file = 0;

	return tmp;
}

int delete_check_farmer(struct check_farmer* p_ch_f) {
	printf("Deleting check_farmer\n");
	free(p_ch_f->fat_item);
	free(p_ch_f->p_boot_record);
	fclose(p_ch_f->file_system);

	free(p_ch_f);
	return 1;
}

struct check_worker* create_check_worker(struct check_farmer* p_ch_f) {
	printf(" Creating check_worker\n");
	struct check_worker* tmp = malloc(sizeof (struct check_worker));

	tmp->p_root_directory = (struct root_directory *) malloc(sizeof (struct root_directory));
	tmp->p_cluster = malloc(sizeof (char) * p_ch_f->p_boot_record->cluster_size);

	tmp->ch_f = p_ch_f;
	tmp->file_seq_num = 0;
	return tmp;
}

int delete_check_worker(struct check_worker* p_ch_w) {
	printf(" Deleting check_worker\n");
	free(p_ch_w->p_root_directory);
	free(p_ch_w->p_cluster);
	free(p_ch_w);

	return 1;
}

int check_farmer_load_next_file(struct check_farmer* ch_f, struct root_directory* rd) {
	int cur_file = ch_f->cur_file;
	if (cur_file >= ch_f->p_boot_record->root_directory_max_entries_count) {
		return 0;
	}

	int file_offset = ch_f->root_directory_offset + cur_file * sizeof (struct root_directory);
	ch_f->cur_file = cur_file + 1;
	fseek(ch_f->file_system, file_offset, SEEK_SET);
	fread(rd, sizeof (struct root_directory), 1, ch_f->file_system);
	return 1;
}

// returns reason why cluster chain broke or 0 if chain continues

int check_farmer_load_next_cluster(struct check_worker* p_ch_w, struct check_farmer* p_ch_f) {
	long cluster_offset = p_ch_f->data_cluster_offset + p_ch_w->next_cluster * sizeof (char) * p_ch_f->p_boot_record->cluster_size;
	fseek(p_ch_f->file_system, cluster_offset, SEEK_SET);
	fread(p_ch_w->p_cluster, sizeof (char) * p_ch_f->p_boot_record->cluster_size, 1, p_ch_f->file_system);
	if (p_ch_w->next_cluster == FAT_BAD_CLUSTER || p_ch_w->next_cluster == FAT_FILE_END) {
		return 0;
	}
	if (p_ch_w->next_cluster > p_ch_f->p_boot_record->cluster_count) {
		return 0;
	}
	return p_ch_f->fat_item[p_ch_w->next_cluster];
}

int check_worker_run(struct check_worker* p_ch_w) {
	int cluster_number;
	while (check_farmer_load_next_file(p_ch_w->ch_f, p_ch_w->p_root_directory)) {
		p_ch_w->file_seq_num++;
		printf("FILE %d \n", p_ch_w->file_seq_num);
		printf("file_name :%s\n", p_ch_w->p_root_directory->file_name);
		printf("file_mod  :%s\n", p_ch_w->p_root_directory->file_mod);
		printf("file_type :%d\n", p_ch_w->p_root_directory->file_type);
		printf("file_size :%d\n", p_ch_w->p_root_directory->file_size);
		printf("1st_clstr :%d\n", p_ch_w->p_root_directory->first_cluster);

		cluster_number = p_ch_w->next_cluster;
		p_ch_w->next_cluster = check_farmer_load_next_cluster(p_ch_w, p_ch_w->ch_f);
		do {
			printf("cl %d\t %s\n", cluster_number, p_ch_w->p_cluster);
			cluster_number = p_ch_w->next_cluster;
		} while ((p_ch_w->next_cluster = check_farmer_load_next_cluster(p_ch_w, p_ch_w->ch_f)));
	}

	return 1;
}
