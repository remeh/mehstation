#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "view/window.h"

int main(int argc, char* argv[]) {
	// Nearly everything is used in the SDL.
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		g_critical("Can't init the SDL: %s", SDL_GetError());
		return 1;
	}
	g_message("SDL init OK.");

	const Window* window = meh_create_window(640, 480, FALSE);

	SDL_Delay(1000);

	atexit(SDL_Quit);
	return 0;
}
