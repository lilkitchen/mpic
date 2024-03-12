/*
 * pic.c
 */

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>

#include "block.h"
#include "rgb.h"
#include "state.h"
#include "texel.h"
#include "pic.h"

extern SDL_Renderer *rendr;

Pic pic;
Texel map[MSIZE][MSIZE];

static inline void surface_pixel_set(SDL_Surface *surf, int x, int y, int r, int g, int b);
static inline Rgb surface_pixel_get(SDL_Surface *surf, int x, int y);

int pic_init(char *file_name)
{
	if (strlen(file_name) > NAME_LIMIT) {
		printf("%s: Name is too long\n", file_name);
		return 1;
	}
	strcpy(pic.name, file_name);
	for (char *c = pic.name; *c; c++)
		if (*c == '.') {
			*c = '\0';
			break;
		}

	pic.img = IMG_Load(file_name);
	if (pic.img == NULL) {
		printf("%s: File not found\n", file_name);
		return 1;
	}

	pic.ogr = (SDL_Rect){
		.w = pic.img->w,
		.h = pic.img->h
	};

	pic.cut = (SDL_Rect){
		.w = pic.img->w / 2,
		.h = pic.img->w / 2
	};

	pic.og = SDL_CreateTextureFromSurface(rendr, pic.img);
	pic.out = SDL_CreateTexture(rendr, SDL_PIXELFORMAT_RGB888,
				    SDL_TEXTUREACCESS_TARGET, MSIZE, MSIZE);
	if (pic.og == NULL || pic.out == NULL) {
		printf("Failed to create objects\n");
		return 1;
	}

	pic.outr = (SDL_Rect){
		.w = MSIZE,
		.h = MSIZE
	};

	state_name_compose();
	if (state_load()) {
		pic_reset();
		pic_limit();
	}

	pic_generate();

	return 0;
}

void pic_quit(void)
{
	SDL_FreeSurface(pic.img);
	SDL_DestroyTexture(pic.og);
	SDL_DestroyTexture(pic.out);
}

void pic_preview(void)
{
	pic_limit();

	SDL_SetRenderTarget(rendr, pic.out);
	SDL_RenderCopy(rendr, pic.og, &pic.cut, &pic.outr);
}

void pic_limit(void)
{
	if (pic.cut.w < 16 || pic.cut.h < 16) {
		pic.cut.w = 16;
		pic.cut.h = 16;
	}

	if (pic.cut.w > pic.ogr.w) {
		pic.cut.w = pic.ogr.w;
		pic.cut.h = pic.ogr.w;
	}

	if (pic.cut.h > pic.ogr.h) {
		pic.cut.w = pic.ogr.h;
		pic.cut.h = pic.ogr.h;
	}

	if (pic.cut.x < 0)
		pic.cut.x = 0;
	if (pic.cut.y < 0)
		pic.cut.y = 0;
	if (pic.cut.x + pic.cut.w >= pic.ogr.w)
		pic.cut.x = pic.ogr.w - pic.cut.w;
	if (pic.cut.y + pic.cut.h >= pic.ogr.h)
		pic.cut.y = pic.ogr.h - pic.cut.h;
}

void pic_reset(void)
{
	for (int i = 0; i < 3; i++)
		pic.boost[i] = 0.0;

	pic.dith_threshold = 0;
}

void pic_generate(void)
{
	SDL_Surface *region = SDL_CreateRGBSurface(0, MSIZE, MSIZE, 24,
						   0, 0, 0, 0);
	int kx = pic.cut.w / MSIZE;
	int ky = pic.cut.h / MSIZE;
	int kk = kx * ky;

	for (int x = 0; x < MSIZE; x++)
		for (int y = 0; y < MSIZE; y++) {
			int r = 0;
			int g = 0;
			int b = 0;

			int px = (float)x / MSIZE * pic.cut.w;
			int py = (float)y / MSIZE * pic.cut.h;
			for (int u = 0; u < kx; u++)
				for (int v = 0; v < ky; v++) {
					Rgb og = surface_pixel_get(pic.img,
								   pic.cut.x + px + u,
								   pic.cut.y + py + v);
					r += og.r;
					g += og.g;
					b += og.b;
				}

			surface_pixel_set(region, x, y,
					  r / kk,
					  g / kk,
					  b / kk);
		}

	SDL_SetRenderTarget(rendr, pic.out);
	for (int x = 0; x < MSIZE; x++)
		for (int y = 0; y < MSIZE; y++) {
			Rgb c = surface_pixel_get(region, x, y);
			rgb_boost(&c, pic.boost);

			Texel nearest[2] = {
				{.diff = 255 * 3},
			};
			for (int i = 0; i < BLOCKS_TOTAL; i++)
				for (int k = 0; k < 3; k++) {
					Rgb bc = blocks[i].color[k];
					int diff = rgb_diff(c, bc);
					if (diff < nearest[0].diff) {
						nearest[1] = nearest[0];
						nearest[0].diff = diff;
						nearest[0].name = blocks[i].name;
						nearest[0].color = bc;
						nearest[0].alt = k - 1;
					}
				}

			int dith = 0;
			if (abs(nearest[0].diff - nearest[1].diff) < pic.dith_threshold)
				dith = (x % 2) == (y % 2);

			map[x][y] = nearest[dith];

			c = nearest[dith].color;

			SDL_SetRenderDrawColor(rendr, c.r, c.g, c.b, 255);
			SDL_RenderDrawPoint(rendr, x, y);
		}

	SDL_FreeSurface(region);
}

#define PRINT_BLOCK(x, y, z, name) \
	do { \
		fprintf(mcfunc, "setblock ~%d ~%d ~%d minecraft:%s\n", x, z, y, name); \
		fprintf(mcfunc_clear, "setblock ~%d ~%d ~%d minecraft:air\n", x, z, y); \
	} while (0)

void pic_export(void)
{
	state_save();

	char name[NAME_LIMIT + EXT_LIMIT];
	char name_clear[NAME_LIMIT + EXT_LIMIT];
	sprintf(name, "%s.mcfunction", pic.name);
	sprintf(name_clear, "%s_clear.mcfunction", pic.name);
	FILE *mcfunc = fopen(name, "w");
	FILE *mcfunc_clear = fopen(name_clear, "w");

	for (int x = 0; x < MSIZE; x++) {
		int min = MSIZE;
		int z = 0;
		for (int y = 0; y < MSIZE; y++) {
			z += map[x][y].alt;
			map[x][y].z = z;
			if (z < min)
				min = z;
		}

		// First raw correction
		min--;

		PRINT_BLOCK(x, -1, map[x][0].z - min - map[x][0].alt, "stone");
		for (int y = 0; y < MSIZE; y++)
			PRINT_BLOCK(x, y, map[x][y].z - min, map[x][y].name);
	}

	fclose(mcfunc);
	fclose(mcfunc_clear);
}

#define SURF_OFFSET \
	(void*)surf->pixels + (y * surf->pitch + x * 3)

static inline void surface_pixel_set(SDL_Surface *surf, int x, int y, int r, int g, int b)
{
	Rgb *c = SURF_OFFSET;
	c->r = r;
	c->g = g;
	c->b = b;
}

static inline Rgb surface_pixel_get(SDL_Surface *surf, int x, int y)
{
	return *(Rgb*)(SURF_OFFSET);
}
