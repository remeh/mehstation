#include "window.h"

/*
 * meh_create_window deals with the creation of the opengl window.
 */
Window* meh_window_create(guint width, guint height, gboolean fullscreen) {
	Window* w = (Window*)malloc(sizeof(Window));	

	w->width = width;
	w->height = height;
	w->fullscreen = fullscreen;

	int flags = SDL_WINDOW_OPENGL;
	if (w->fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	/* Create the SDL window */
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

	/* Attach a renderer to the window. */
	w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, -1, 0);
	if (w->sdl_renderer == NULL) {
		g_critical("Can't attach the renderer: %s", SDL_GetError());
		return NULL;
	}

	g_message("Window %d:%d %s created.", w->width, w->height, (w->fullscreen == TRUE ? "fullscreen" : "windowed"));
	return w;
}

/*
 * meh_destroy_window frees the resources of the given window.
 */
gboolean meh_window_destroy(Window* window) {
	g_assert(window != NULL);

	if (window->sdl_window != NULL) {
		SDL_DestroyWindow(window->sdl_window);
		window->sdl_window = NULL;
	}
	if (window->sdl_renderer != NULL) {
		SDL_DestroyRenderer(window->sdl_renderer);
		window->sdl_renderer = NULL;
	}

	free(window);

	return TRUE;
}

/*
 * meh_window_clear clears with the given color.
 */
void meh_window_clear(Window* window, SDL_Color color) {
	g_assert(window != NULL);
	
	SDL_SetRenderDrawColor(window->sdl_renderer, color.r, color.g, color.b, 255);
	SDL_RenderClear(window->sdl_renderer);
}

/*
 * meh_window_render pushes the changes done in the renderer to the window's surface.
 */
void meh_window_render(Window* window) {
	SDL_RenderPresent(window->sdl_renderer);
}

/*
 * meh_window_render_texture renders the given texture at the given position.
 */
void meh_window_render_texture(Window* window, SDL_Texture* texture, SDL_Rect viewport) {
	g_assert(window != NULL);
	g_assert(texture != NULL);

	SDL_RenderSetViewport(window->sdl_renderer, &viewport);
	SDL_RenderCopy(window->sdl_renderer, texture, NULL, NULL);
}
