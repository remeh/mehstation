#include "window.h"

const Window* meh_create_window(guint width, guint height, gboolean fullscreen) {
	Window* w = (Window*)malloc(sizeof(Window));	

	w->width = width;
	w->height = height;
	w->fullscreen = fullscreen;

	int flags = SDL_WINDOW_OPENGL;
	if (w->fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	w->sdl_window = SDL_CreateWindow(
			"mehstation",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			w->width,
			w->height,
			flags
	);

	if (w->sdl_window == NULL) {
		g_critical("Can't open the window: %s", SDL_GetError());
		return NULL;
	}

	g_message("Window created.");
	return w;
}
