#include <stdlib.h>

#include "length-check.h"

struct check_farmer* create_check_farmer(FILE* p_file, struct boot_record *p_boot_record) {
	printf("Creating check_farmer\n");
	struct check_farmer* tmp = malloc(sizeof (struct check_farmer));
	tmp->fat_item = malloc(p_boot_record->cluster_count * p_boot_record->fat_copies * sizeof (unsigned int));
	
	int root_directory_offset = 0, data_cluster_offset = 0;

	// nacteni FAT
	fread(tmp->fat_item, sizeof (unsigned int), p_boot_record->cluster_count * p_boot_record->fat_copies, p_file);
	root_directory_offset = ftell(p_file);
	
	data_cluster_offset = root_directory_offset + p_boot_record->root_directory_max_entries_count * sizeof (struct root_directory);

	printf("root_directory_offset :%d\n", root_directory_offset);
	printf("data_cluster_offset :%d\n", data_cluster_offset);

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
	free(p_ch_f);
	return 1;
}

struct check_worker* create_check_worker(struct check_farmer* p_ch_f) {
	printf(" Creating check_worker\n");
	struct check_worker* tmp = malloc(sizeof (struct check_worker));

	tmp->p_root_directory = (struct root_directory *) malloc(sizeof (struct root_directory));
	tmp->ch_f = p_ch_f;
	return tmp;
}

int delete_check_worker(struct check_worker* p_ch_w) {
	printf(" Deleting check_worker\n");
	free(p_ch_w->p_root_directory);
	free(p_ch_w);

	return 1;
}

struct root_directory* check_farmer_load_next_file(struct check_farmer* ch_f, struct root_directory* rd) {
	int cur_file = ch_f->cur_file;
	if (cur_file >= ch_f->p_boot_record->root_directory_max_entries_count) {
		return NULL;
	}

	int file_offset = ch_f->root_directory_offset + cur_file * sizeof (struct root_directory);

	fseek(ch_f->file_system, file_offset, SEEK_SET);
	fread(rd, sizeof (struct root_directory), 1, ch_f->file_system);
	printf("FILE %d \n", cur_file);
	printf("file_name :%s\n", rd->file_name);
	printf("file_mod  :%s\n", rd->file_mod);
	printf("file_type :%d\n", rd->file_type);
	printf("file_size :%d\n", rd->file_size);
	printf("1st_clstr :%d\n", rd->first_cluster);
}