#pragma once

#include <glib.h>
#include <SDL2/SDL.h>

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
void meh_window_destroy(Window* window);
void meh_window_clear(Window* window, SDL_Color color);
void meh_window_render(Window* window);
void meh_window_render_texture(Window* window, SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dst);
int meh_window_render_text(Window* window, const Font* font, const char* text, SDL_Color color, int x, int y, float max_width);
SDL_Texture* meh_window_render_text_texture(Window* window, const Font* font, const char* text, SDL_Color color, float max_width);
float meh_window_convert_width(Window* window, float normalized_x);
float meh_window_convert_height(Window* window, float normalized_y);
