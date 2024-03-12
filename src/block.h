/*
 * block.h
 */

#ifndef MPIC_BLOCK_H_
#define MPIC_BLOCK_H_

#include "rgb.h"

#define BLOCKS_TOTAL 59

typedef struct block Block;

struct block {
	char *name;
	Rgb color[3];
};

extern Block blocks[BLOCKS_TOTAL];

#endif
