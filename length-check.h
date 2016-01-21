#ifndef LENGTH_CHECK_H
#define	LENGTH_CHECK_H

#include "fat-operator.h"

struct check_farmer {
	int n;
};

struct check_worker {
	int n;
};

struct check_farmer* create_check_farmer();

int delete_check_farmer(struct check_farmer* p_c_f);


struct check_worker* create_check_worker();

int delete_check_worker(struct check_worker* p_c_f);



#endif	/* LENGTH_CHECK_H */
