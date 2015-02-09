#include "glib-2.0/glib.h"

#include "text.h"

/*
 * meh_font_open reads the given filename as a font with the given size.
 */
Font* meh_font_open(const char* filename, guint size) {
	Font* font = NULL;

	TTF_Font* sdl_font = TTF_OpenFont(filename, size);
	if (sdl_font == NULL) {
		g_warning("Unable to open the font '%s' in size '%d' : %s\n", filename, size, SDL_GetError());
		return NULL;
	}

	font = g_new(Font, 1);
	font->sdl_font = sdl_font;

	return font;
}

/*
 * meh_font_destroy frees the resources of the given font.
 */
void meh_font_destroy(Font* font) {
	g_assert(font != NULL);

	TTF_CloseFont(font->sdl_font);

	g_free(font);
}

/*
 * meh_font_render_on_surface uses the given font to render a text in the given
 * color on a surface.
 *
 * The returned surface should be freed by the caller.
 */
SDL_Surface* meh_font_render_on_surface(Font* font, const char* text, SDL_Color color) {
	g_assert(font != NULL);
	g_assert(text != NULL);

	SDL_Surface* surface = TTF_RenderText_Solid(font->sdl_font, text, color);

	if (surface == NULL) {
		g_warning("Unable to render the text '%s' on a SDL_Surface: %s\n", SDL_GetError());
		return NULL;
	}

	return surface;
}
