#pragma once

#include "glib-2.0/glib.h"
#include "SDL2/SDL.h"

/*
 * Main window.
 */
typedef struct {
	guint width;
	guint height;
	gboolean fullscreen;
	SDL_Window* sdl_window;
} Window;

Window* meh_create_window(guint width, guint height, gboolean fullscreen);
