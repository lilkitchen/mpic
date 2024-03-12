/*
 * state.c
 */

#include <stdio.h>
#include <string.h>

#include "pic.h"
#include "state.h"

void state_name_compose(void)
{
	sprintf(pic.state_name, "%s.mpic", pic.name);

}

#define RW(rw, m) \
	FILE *file = fopen(pic.state_name, m); \
	if (file == NULL) \
		return 1; \
	f##rw(&pic.cut, sizeof(pic.cut), 1, file); \
	f##rw(&pic.dith_threshold, sizeof(pic.dith_threshold), 1, file); \
	f##rw(&pic.boost, sizeof(pic.boost), 1, file); \
	fclose(file); \
	return 0

int state_save(void)
{
	RW(write, "wb");
}

int state_load(void)
{
	RW(read, "rb");
}
