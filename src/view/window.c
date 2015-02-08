#include "window.h"

/*
 * meh_create_window deals with the creation of the opengl window.
 */
Window* meh_create_window(guint width, guint height, gboolean fullscreen) {
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

	g_message("Window %d:%d %s created.", w->width, w->height, (w->fullscreen == TRUE ? "fullscreen" : "windowed"));
	return w;
}

/*
 * meh_destroy_window frees the resources of the given window.
 */
gboolean meh_destroy_window(Window* window) {
	if (window == NULL) {
		return FALSE;
	}

	SDL_DestroyWindow(window->sdl_window);
	free(window);

	return TRUE;
}
