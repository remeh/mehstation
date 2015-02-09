#pragma once

#include "glib-2.0/glib.h"
#include "SDL2/SDL.h"

#include "view/text.h"

/*
 * Main window.
 */
typedef struct {
	guint width;
	guint height;
	gboolean fullscreen;
	SDL_Window* sdl_window;
	SDL_Renderer* sdl_renderer;
} Window;

Window* meh_window_create(guint width, guint height, gboolean fullscreen);
gboolean meh_window_destroy(Window* window);
void meh_window_clear(Window* window, SDL_Color color);

void meh_window_render(Window* window);
void meh_window_render_texture(Window* window, SDL_Texture* texture, SDL_Rect viewport);
int meh_window_render_text(Window* window, Font* font, const char* text, SDL_Color color, int x, int y);
