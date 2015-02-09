#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

typedef struct {
	TTF_Font* sdl_font;
} Font;

/*
 * meh_open_font reads the given filename as a font with the given size.
 */
Font* meh_open_font(const char* filename, uint size);

/*
 * meh_destroy_font frees the resources of the given font.
 */
void meh_destroy_font(Font* font);
