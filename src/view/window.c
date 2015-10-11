#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "view/window.h"
#include "view/text.h"
#include "system/consts.h"

static gboolean meh_window_has_accelerated(void);

/*
 * meh_create_window deals with the creation of the opengl window.
 */
Window* meh_window_create(guint width, guint height, gboolean fullscreen, gboolean force_software) {
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


	/* Looks whether or any driver support acceleration */

	/* Attach a renderer to the window. */
	if (meh_window_has_accelerated() && !force_software) {
		w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, -1, SDL_RENDERER_ACCELERATED);
	} else {
		g_message("No hardware acceleration. Trying to fallback on the software renderer.");
		w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, -1, SDL_RENDERER_SOFTWARE);
	}

	if (w->sdl_renderer == NULL) {
		g_message("Can't create any renderer: %s", SDL_GetError());
		return NULL;
	}

	/* Uses SDL2 auto-scaling system. */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(w->sdl_renderer, w->width, w->height);

	g_message("Window %d:%d %s created.", w->width, w->height, (w->fullscreen == TRUE ? "fullscreen" : "windowed"));
	return w;
}

/*
 * meh_window_has_accelerated returns if any accelerated renderer
 * support hardware acceleration.
 */
gboolean meh_window_has_accelerated(void) {
	for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		if (info.flags & SDL_RENDERER_ACCELERATED) {
			return TRUE;
		}
	}
	return FALSE;
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
void meh_window_render_texture(Window* window, SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dst) {
	g_assert(window != NULL);
	g_assert(texture != NULL);

	SDL_RenderCopyEx(window->sdl_renderer, texture, src, dst, 0.0, NULL, SDL_FLIP_NONE);
}

/*
 * meh_window_render_text directly renders text on the rendered with the given font
 * at the given position.
 * To render on a single line : max_width should be equal to -1.0f
 * If max_width != 1.0f, the text is rendered wrapped.
 * Returns:
 *  0 if everything succeed
 *  1 if an error occurred
 */
int meh_window_render_text(Window* window, const Font* font, const char* text, SDL_Color color, int x, int y, float max_width) {
	g_assert(window != NULL);
	g_assert(font != NULL);

	/* Write the text on a texture. */
	SDL_Texture* texture = meh_window_render_text_texture(window, font, text, color, max_width);

	/* Renders at the good position */
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_Rect dst = { x, y, w, h };
	meh_window_render_texture(window, texture, NULL, &dst);

	/* Free the texture. */
	SDL_DestroyTexture(texture);

	return 0;
}

/*
 * meh_window_render_text_texture renders the given text with the given font on a SDL_Texture
 * and returns it.
 */
SDL_Texture* meh_window_render_text_texture(Window* window, const Font* font, const char* text, SDL_Color color, float max_width) {
	g_assert(window != NULL);
	g_assert(font != NULL);

	/*
	 * If the text is empty, render a space.
	 * NOTE bit of a trick to avoid error while using render_text_texture
	 */
	const char* rendered_text = " ";
	if (text != NULL && strlen(text) > 0) {
		rendered_text = text;
	}

	SDL_Texture* texture = meh_font_render_on_texture(window->sdl_renderer, font, rendered_text, color, max_width);

	if (texture == NULL) {
		g_critical("Can't render text on a texture.");
		return NULL;
	}

	return texture;
}

float meh_window_convert_width(Window* window, float fake_x) {
	return (fake_x/MEH_FAKE_WIDTH) * (float)window->width;
}

float meh_window_convert_height(Window* window, float fake_y) {
	return (fake_y/MEH_FAKE_HEIGHT) * (float)window->height;
}
