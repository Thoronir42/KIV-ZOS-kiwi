#include <stdlib.h>

#include "length-check.h"

struct check_farmer* create_farmer() {
	struct check_farmer* tmp = malloc(sizeof (struct check_farmer));

	return tmp;
}

int delete_check_farmer(struct check_farmer* p_c_f) {
	free(p_c_f);
	return 1;
}

struct check_worker* create_check_worker() {
	struct check_worker* tmp = malloc(sizeof (struct check_worker));

	return tmp;
}

int delete_check_worker(struct check_worker* p_c_w) {
	free(p_c_w);
	
	return 1;
}