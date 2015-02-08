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
	const SDL_Window* sdl_window;
} Window;

const Window* meh_create_window(guint width, guint height, gboolean fullscreen);
