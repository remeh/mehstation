#include "glib-2.0/glib.h"
#include "view/window.h"
#include "view/text.h"

/*
 * meh_create_window deals with the creation of the opengl window.
 */
Window* meh_window_create(guint width, guint height, gboolean fullscreen) {
	Window* w = g_new(Window, 1);

	w->width = width;
	w->height = height;
	w->fullscreen = fullscreen;

	int flags = SDL_WINDOW_OPENGL;
	if (w->fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
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

	/* Uses SDL2 auto-scaling system. */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(w->sdl_renderer, w->width, w->height);

	g_message("Window %d:%d %s created.", w->width, w->height, (w->fullscreen == TRUE ? "fullscreen" : "windowed"));
	return w;
}

/*
 * meh_destroy_window frees the resources of the given window.
 */
void meh_window_destroy(Window* window) {
	g_assert(window != NULL);

	if (window->sdl_window != NULL) {
		SDL_DestroyWindow(window->sdl_window);
		window->sdl_window = NULL;
	}
	if (window->sdl_renderer != NULL) {
		SDL_DestroyRenderer(window->sdl_renderer);
		window->sdl_renderer = NULL;
	}

	g_free(window);
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

/*
 * meh_window_render_text directly renders text on the rendered with the given font
 * at the given position.
 * Returns:
 *  0 if everything succeed
 *  1 if an error occurred
 */
int meh_window_render_text(const Window* window, const Font* font, const char* text, SDL_Color color, int x, int y) {
	g_assert(window != NULL);
	g_assert(font != NULL);
	g_assert(text != NULL);

	if (text == '\0') {
		return 1;
	}

	SDL_Texture* texture = meh_font_render_on_texture(window->sdl_renderer, font, text, color);
	if (texture == NULL) {
		g_critical("Can't render text on the window.\n");
		return 1;
	}

	/* Renders at the good position */
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_Rect viewport = { x, y, w, h };
	meh_window_render_texture(window, texture, viewport);

	/* Free the texture. */
	SDL_DestroyTexture(texture);

	return 0;
}
