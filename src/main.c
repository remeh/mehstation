#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "view/window.h"
#include "system/settings.h"

int main(int argc, char* argv[]) {
	/* Nearly everything is used in the SDL. */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		g_critical("Can't init the SDL: %s", SDL_GetError());
		return 1;
	}
	g_message("SDL init OK.");
	atexit(SDL_Quit);

	/* Read the settings */
	Settings settings;
	meh_read_settings(&settings, "mehstation.conf");

	/* Open the main window */
	Window* window = meh_create_window(settings.width, settings.height, settings.fullscreen);

	SDL_Delay(1000);

	/* Free the resource */
	meh_destroy_window(window);
	window = NULL;

	return 0;
}
