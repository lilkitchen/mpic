/*
 * texel.h
 */

#ifndef MPIC_TEXEL_H_
#define MPIC_TEXEL_H_

#include "rgb.h"

typedef struct texel Texel;

struct texel {
	char *name;
	Rgb color;
	int diff;
	int alt;
	int z;
};

#endif
