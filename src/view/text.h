#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

typedef struct {
	TTF_Font* sdl_font;
} Font;

Font* meh_open_font(const char* filename, uint size);

void meh_destroy_font(Font* font);
