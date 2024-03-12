/*
 * rgb.h
 */

#ifndef MPIC_RGB_H_
#define MPIC_RGB_H_

typedef union rgb Rgb;

union rgb {
	struct {
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};
	unsigned char v[3];
};

int rgb_diff(Rgb x, Rgb y);
void rgb_boost(Rgb *c, float *boost);

#endif
