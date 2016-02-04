#include <stdlib.h>
#include <pthread.h>

#include "cluster-shake.h"

#ifndef FAT_UNUSED
#define FAT_UNUSED 65535
#define FAT_FILE_END 65534
#define FAT_BAD_CLUSTER 65533
#endif

const int CL_CHUNK_SIZE = 4;
const int NO_SHAKE_JOB = -1;

int shake_analyze_fat(struct shake_farmer *p_s_f) {
	int chunk_last_index = p_s_f->CLUSTER_CHUNK_SIZE - 1;

	int i, cl_ch_i, non_empty, bad_clusters, mod, cluster_last_non_empty;
	cl_ch_i = non_empty = bad_clusters = 0;

	int cur_FAT_item;

	printf("FAT analyzation starting with chunk size %d... ", p_s_f->CLUSTER_CHUNK_SIZE);

	for (i = 0; i < p_s_f->p_boot_record->cluster_count; i++) {
		cur_FAT_item = p_s_f->FAT[i];
		if (cur_FAT_item == FAT_UNUSED) {
			continue;
		}
		if (cur_FAT_item == FAT_BAD_CLUSTER) {
			bad_clusters++;
			continue;
		}
		cluster_last_non_empty = i;

		mod = non_empty++ % p_s_f->CLUSTER_CHUNK_SIZE;
		if ((mod) == 0) {
			printf("cl_ch_start on %04d\n", cluster_last_non_empty);
			p_s_f->cluster_chunk_read_beginings[cl_ch_i] = cluster_last_non_empty;
		} else if (mod == chunk_last_index) {
			printf("cl_ch_end   on %04d\n", cluster_last_non_empty);
			p_s_f->cluster_chunk_read_ends[cl_ch_i++] = cluster_last_non_empty;
		}
		if (cur_FAT_item != FAT_FILE_END) {
			p_s_f->FAT_rev[cur_FAT_item] = cluster_last_non_empty;
		}
	}

	mod = non_empty % p_s_f->CLUSTER_CHUNK_SIZE;
	p_s_f->cluster_chunks_not_empty = (non_empty / p_s_f->CLUSTER_CHUNK_SIZE);
	if (mod) {
		p_s_f->cluster_chunks_not_empty++;
		p_s_f->cluster_chunk_last_size = mod;
		p_s_f->cluster_chunk_read_ends[cl_ch_i] = cluster_last_non_empty;
		printf("cl_ch_end   on %04d\n", cluster_last_non_empty);
	} else {
		p_s_f->cluster_chunk_last_size = p_s_f->CLUSTER_CHUNK_SIZE;
	}

	printf("done\n");
	printf("Non empty clusters: %04d, bad clusters: %02d\n", non_empty, bad_clusters);
	printf("Last chunk: %02d\tLast chunk size: %d\n\n",
			p_s_f->cluster_chunks_not_empty, p_s_f->cluster_chunk_last_size);
	return 0;

}

int write_shaken_headder(struct shake_farmer *p_s_f, char *file_name) {
	FILE* p_file;
	int i;

	unlink(file_name);
	p_file = fopen(file_name, "w");
	fwrite(p_s_f->p_boot_record, sizeof (struct boot_record), 1, p_file);
	for (i = 0; i < p_s_f->p_boot_record->fat_copies; i++) {
		fwrite(p_s_f->FAT, sizeof (unsigned int), p_s_f->p_boot_record->cluster_count, p_file);
	}
	fwrite(p_s_f->p_root_directory, sizeof (struct root_directory), p_s_f->p_boot_record->root_directory_max_entries_count, p_file);
	fwrite(p_s_f->cluster_content, sizeof (char) * p_s_f->p_boot_record->cluster_size, p_s_f->p_boot_record->cluster_count, p_file);
}

struct shake_farmer *create_shake_farmer(char* FS_path) {
	struct shake_farmer* tmp = malloc(sizeof (struct shake_farmer));
	FILE* p_file;

	// otevru soubor a pro jistotu skocim na zacatek           
	p_file = fopen(FS_path, "r");
	fseek(p_file, 0, SEEK_SET);

	tmp->FS_path = FS_path;
	tmp->file_system = p_file;

	// inicializace boot record
	tmp->p_boot_record = malloc(sizeof (struct boot_record));
	fread(tmp->p_boot_record, sizeof (struct boot_record), 1, tmp->file_system);

	// inicializace a nacteni FAT
	tmp->FAT = malloc(tmp->p_boot_record->cluster_count * sizeof (unsigned int));
	tmp->FAT_rev = malloc(tmp->p_boot_record->cluster_count * sizeof (unsigned int));
	tmp->offset_fat = ftell(tmp->file_system);
	fread(tmp->FAT, sizeof (unsigned int), tmp->p_boot_record->cluster_count, tmp->file_system);

	// inicializace cluster mutexu
	tmp->lock_cluster_chunk_counter = malloc(sizeof (pthread_mutex_t));
	pthread_mutex_init(tmp->lock_cluster_chunk_counter, NULL);

	// inicializace a nacteni root directory
	tmp->offset_root_directory = ftell(tmp->file_system);
	tmp->p_root_directory = malloc(sizeof (struct root_directory) * tmp->p_boot_record->root_directory_max_entries_count);
	fread(tmp->p_root_directory, sizeof (struct root_directory), tmp->p_boot_record->root_directory_max_entries_count, tmp->file_system);

	// inicializace a nacteni datovych clusteru
	tmp->offset_data_cluster = ftell(tmp->file_system);
	tmp->cluster_content = malloc(sizeof (char) * tmp->p_boot_record->cluster_size * tmp->p_boot_record->cluster_count);
	fread(tmp->cluster_content, sizeof (char) * tmp->p_boot_record->cluster_size, tmp->p_boot_record->cluster_count, tmp->file_system);

	// inicializace promennych
	*(int *) &tmp->CLUSTER_CHUNK_SIZE = CL_CHUNK_SIZE;

	tmp->cluster_chunk_read_beginings = malloc(sizeof (int) * (tmp->p_boot_record->cluster_count / tmp->CLUSTER_CHUNK_SIZE));
	tmp->cluster_chunk_read_ends = malloc(sizeof (int) * (tmp->p_boot_record->cluster_count / tmp->CLUSTER_CHUNK_SIZE));
	tmp->cluster_chunk_current = 0;


	write_shaken_headder(tmp, "shaken.fat");

	return tmp;
}

int delete_shake_farmer(struct shake_farmer *p_s_f) {
	free(p_s_f->p_boot_record);
	free(p_s_f->FAT);
	free(p_s_f->FAT_rev);
	free(p_s_f->p_root_directory);
	free(p_s_f->cluster_content);

	free(p_s_f->cluster_chunk_read_beginings);
	free(p_s_f->cluster_chunk_read_ends);


	pthread_mutex_destroy(p_s_f->lock_cluster_chunk_counter);
	free(p_s_f->lock_cluster_chunk_counter);

	fclose(p_s_f->file_system);

	free(p_s_f);
	return 1;
}

struct shake_worker *create_shake_worker(struct shake_farmer *p_s_f, int w_id) {
	struct shake_worker* tmp = malloc(sizeof (struct shake_worker));

	tmp->s_f = p_s_f;
	tmp->worker_id = w_id;

	tmp->file_system_operator = fopen("shaken.fat", "r+");
	//tmp->file_system_operator = fopen(p_s_f->FS_path, "r+");

	return tmp;
}

int delete_shake_worker(struct shake_worker *p_s_w) {
	fclose(p_s_w->file_system_operator);
	free(p_s_w);

	return 1;
}

int shake_next_cluster_chunk(struct shake_worker* p_s_w, struct shake_farmer * p_s_f) {
	if (p_s_f->cluster_chunk_current >= p_s_f->cluster_chunks_not_empty) {
		p_s_w->assigned_cluster_chunk = NO_SHAKE_JOB;
		return 0;
	}
	pthread_mutex_lock(p_s_f->lock_cluster_chunk_counter);
	p_s_w->assigned_cluster_chunk = p_s_f->cluster_chunk_current;
	p_s_f->cluster_chunk_current++;
	pthread_mutex_unlock(p_s_f->lock_cluster_chunk_counter);
	return 1;
}

void *shake_worker_run(struct shake_worker * p_s_w) {
	struct shake_farmer *p_s_f = p_s_w->s_f;
	int put_index, search_index, search_item;
	int chunk_offset, search_chunk_start, search_chunk_end;

	printf("Running shake worker %02d with chunk size %d\n", p_s_w->worker_id, p_s_f->CLUSTER_CHUNK_SIZE);

	while (shake_next_cluster_chunk(p_s_w, p_s_w->s_f)) { // Dokud jsou nepřesunuté chunky
		if (p_s_w->assigned_cluster_chunk == NO_SHAKE_JOB) {
			break;
		}

		/*printf("(W%02d-CH%02d) got chunk: <%04d, %04d>\n",
				p_s_w->worker_id, p_s_w->assigned_cluster_chunk,
				p_s_f->cluster_chunk_read_beginings[p_s_w->assigned_cluster_chunk],
				p_s_f->cluster_chunk_read_ends[p_s_w->assigned_cluster_chunk]);*/

		chunk_offset = p_s_w->assigned_cluster_chunk * p_s_f->CLUSTER_CHUNK_SIZE;
		search_chunk_start = p_s_f->cluster_chunk_read_beginings[p_s_w->assigned_cluster_chunk];
		search_chunk_end = p_s_f->cluster_chunk_read_ends[p_s_w->assigned_cluster_chunk];
		search_index = 0;
		for (put_index = 0; put_index < p_s_f->CLUSTER_CHUNK_SIZE; put_index++) { // přesouvej nalezené clustery
			if (p_s_w->assigned_cluster_chunk == p_s_f->cluster_chunks_not_empty - 1 && put_index >= p_s_f->cluster_chunk_last_size ) {
				printf("Last chunk, last cluster.\n");
				break;
			}
			
			// prohledani FAT podle cluster chunku
			search_item = p_s_f->FAT[search_chunk_start + search_index];
			while (search_item == FAT_BAD_CLUSTER || search_item == FAT_UNUSED) { // hledej neprázdné clustery
				// overeni ze se prohledava spravny cluster chunk
				if (search_index > search_chunk_end) {
					printf("(W%02d-CH%02d): SI = %04d exceeded SCE = %04d\n",
							p_s_w->worker_id, p_s_w->assigned_cluster_chunk,
							search_index, p_s_f->cluster_chunk_read_ends[p_s_w->assigned_cluster_chunk]);
					break;
				}
				// dalsi polozka FAT
				search_item = p_s_f->FAT[search_chunk_start + ++search_index];
			}
			printf("(W%02d-CH%02d): B = %04d\tP = %d[%04d+%d]: %05d \n",
					p_s_w->worker_id, p_s_w->assigned_cluster_chunk,
					p_s_f->cluster_chunk_read_beginings[p_s_w->assigned_cluster_chunk],
					chunk_offset + put_index,
					search_chunk_start, search_index, search_item);


		}
	}
}
