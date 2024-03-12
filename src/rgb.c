/*
 * rgb.c
 */

#include <stdlib.h>

#include "rgb.h"

int rgb_diff(Rgb x, Rgb y)
{
	return abs(x.r - y.r) + abs(x.g - y.g) + abs(x.b - y.b);
}

void rgb_boost(Rgb *c, float *boost)
{
	for (int i = 0; i < 3; i++) {
		int f = c->v[i] * (1.0 + boost[i]);
		if (f > 255)
			f = 255;

		c->v[i] = f;
	}
}
