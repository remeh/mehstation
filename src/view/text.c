#include "glib-2.0/glib.h"

#include "view/text.h"

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
	font->size = size;

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
 * If max_width == -1.0f, render to a single line, otherwise render wrapped.
 *
 * The returned texture should be freed by the caller.
 */
SDL_Texture* meh_font_render_on_texture(SDL_Renderer* renderer, const Font* font, const char* text, SDL_Color color, float max_width) {
	g_assert(font != NULL);
	g_assert(text != NULL);

	SDL_Surface* surface = NULL;
	/* either render on a single line or on multiple lines. */
	if (max_width == -1.0f) {
		surface = TTF_RenderUTF8_Blended(font->sdl_font, text, color);
	} else {
		surface = TTF_RenderUTF8_Blended_Wrapped(font->sdl_font, text, color, max_width);
	}

	SDL_Texture* texture = NULL;

	if (surface == NULL) {
		g_warning("Unable to render the text '%s' on a SDL_Surface: %s", text, SDL_GetError());
		return NULL;
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (texture == NULL) {
		g_critical("Unable to create a texture from the text '%s' : %s", text, SDL_GetError());
	}
	
	return texture;
}
