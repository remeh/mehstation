#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glib-2.0/glib.h>

#include "view/image.h"

/*
 * meh_image_load_file loads the given file as a texture.
 * The texture should be freed by the caller.
 */
SDL_Texture* meh_image_load_file(SDL_Renderer* renderer, const char* filename) {
	g_assert(renderer != NULL);
	g_assert(filename != NULL);

	SDL_Surface* surface = IMG_Load(filename);
	if (surface == NULL) {
		g_critical("Can't load the image '%s' : %s", filename, IMG_GetError());
		return NULL;
	}
	
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (texture == NULL) {
		g_critical("Unable to create a texture from the image '%s' : %s", filename, SDL_GetError());
	}
	
	return texture;
}
