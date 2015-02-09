#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

typedef struct {
	TTF_Font* sdl_font;
} Font;

Font* meh_font_open(const char* filename, guint size);

void meh_font_destroy(Font* font);

