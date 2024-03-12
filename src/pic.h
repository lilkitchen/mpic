/*
 * pic.h
 */

#ifndef MPIC_PIC_H_
#define MPIC_PIC_H_

#include <SDL2/SDL.h>

#define NAME_LIMIT 128
#define EXT_LIMIT 20

#define MSIZE 128

typedef struct pic Pic;

extern struct pic {
	char name[NAME_LIMIT];
	char state_name[NAME_LIMIT + EXT_LIMIT];

	SDL_Surface *img;
	SDL_Texture *og;
	SDL_Rect ogr;
	SDL_Rect cut;

	SDL_Texture *out;
	SDL_Rect outr;

	int dith_threshold;
	float boost[3];
} pic;

int pic_init(char *file_name);
void pic_quit(void);

void pic_preview(void);
void pic_limit(void);
void pic_reset(void);

void pic_generate(void);
void pic_export(void);

#endif
