/*
 * mpic.c
 */

#ifdef __WIN32__
#include <windows.h>
#endif

#include <stdio.h>

#include "pic.h"

#define FPS 60
#define FRAME_DELAY 1000.0 / FPS

SDL_Window *window;
SDL_Renderer *rendr;

static char quit;

static SDL_Rect cut_last;
static int mx_press;
static int my_press;

#ifdef __WIN32__
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int __argc, char **__argv)
#endif
{
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) >= 0) {
		window = SDL_CreateWindow("MPic",
					  SDL_WINDOWPOS_CENTERED,
					  SDL_WINDOWPOS_CENTERED,
					  1024, 1024,
					  SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
		if (window == NULL)
			return 1;
		rendr = SDL_CreateRenderer(window, -1,
					   SDL_RENDERER_ACCELERATED |
					   SDL_RENDERER_PRESENTVSYNC);
	} else {
		printf("Could not initialize SDL\n");
		return 1;
	}

	SDL_SetRenderDrawBlendMode(rendr, SDL_BLENDMODE_BLEND);

	char *file_name = "picture.jpg";
	if (__argc > 1)
		file_name = __argv[1];

	if (pic_init(file_name))
		return 1;

	SDL_SetWindowSize(window, pic.ogr.w, pic.ogr.h);

	while (!quit) {
		const unsigned char *kb_state = SDL_GetKeyboardState(NULL);
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
				mx_press = event.button.x;
				my_press = event.button.y;
				cut_last = pic.cut;
				break;

			case SDL_MOUSEBUTTONUP:
				pic_generate();
				break;

			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON_LMASK) {
					pic.cut.x = cut_last.x + event.motion.x - mx_press;
					pic.cut.y = cut_last.y + event.motion.y - my_press;
					pic_preview();

				} else if (event.motion.state & SDL_BUTTON_RMASK) {
					int diff = event.motion.x - mx_press +
						   event.motion.y - my_press;
					pic.cut.w = cut_last.w + diff;
					pic.cut.h = cut_last.h + diff;
					if (pic.cut.w < MSIZE) {
						pic.cut.w = MSIZE;
						pic.cut.h = MSIZE;
					}
					pic_preview();
				}
				break;
#define CASE_BOOST(k, n) \
	if (0) { \
	case SDL_SCANCODE_##k: \
		if (kb_state[SDL_SCANCODE_LSHIFT] || \
		    kb_state[SDL_SCANCODE_RSHIFT]) { \
			pic.boost[n] -= 0.01; \
		} else { \
			pic.boost[n] += 0.01; \
		} \
	}

			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_LEFTBRACKET:
					if (pic.dith_threshold > 0)
						pic.dith_threshold -= 1;
				if (0)
				case SDL_SCANCODE_RIGHTBRACKET:
					pic.dith_threshold += 1;
				CASE_BOOST(R, 0);
				CASE_BOOST(G, 1);
				CASE_BOOST(B, 2);
				if (0)
				case SDL_SCANCODE_BACKSPACE:
					pic_reset();
				pic_generate();
				break;

				case SDL_SCANCODE_RETURN:
					pic_generate();
					pic_export();
					break;

				case SDL_SCANCODE_Q:
					quit = 1;
					break;

				default:
					break;
				}
				break;

			case SDL_QUIT:
				quit = 1;
				break;
			default:
				break;
			}
		}

		int ts = SDL_GetTicks();

		SDL_SetRenderTarget(rendr, NULL);

		SDL_SetRenderDrawColor(rendr, 40, 40, 40, 255);
		SDL_RenderClear(rendr);

		SDL_RenderCopy(rendr, pic.og, &pic.ogr, &pic.ogr);
		SDL_RenderCopy(rendr, pic.out, &pic.outr, &pic.cut);
		SDL_SetRenderDrawColor(rendr, 240, 20, 20, 255);
		SDL_RenderDrawRect(rendr, &pic.cut);

		SDL_SetRenderDrawColor(rendr, 240, 20, 20, 120);
		int third = pic.cut.w / 3;
		SDL_Rect frame = {
			.x = pic.cut.x + third,
			.y = pic.cut.y,
			.w = third,
			.h = pic.cut.h
		};
		SDL_RenderDrawRect(rendr, &frame);

		frame = (SDL_Rect){
			.x = pic.cut.x,
			.y = pic.cut.y + third,
			.w = pic.cut.w,
			.h = third
		};
		SDL_RenderDrawRect(rendr, &frame);

		SDL_RenderPresent(rendr);

		int frame_time = SDL_GetTicks() - ts;
		if (frame_time < FRAME_DELAY)
			SDL_Delay((int)(FRAME_DELAY - frame_time));
	}

	pic_quit();

	SDL_DestroyRenderer(rendr);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
