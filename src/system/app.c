#include "glib-2.0/glib.h"
#include "system/app.h"

App* meh_app_create() {
	return g_new(App, 1);
}

int meh_app_init(App* app) {
	g_assert(app != NULL);

	/* Nearly everything is used in the SDL. */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		g_critical("Can't init the SDL: %s", SDL_GetError());
		return 1;
	}
	if (TTF_Init() == -1) {
		g_critical("TTF can't initialize: %s\n", TTF_GetError());
		return 1;
	}

	g_message("SDL init OK.");
	atexit(SDL_Quit);

	/* Read the settings */
	Settings settings;
	meh_settings_read(&settings, "mehstation.conf");
	app->settings = settings;

	/* Open the main window */
	Window* window = meh_window_create(settings.width, settings.height, settings.fullscreen);
	app->window = window;

	/* Opens some font. */
	Font* font = meh_font_open("/usr/share/fonts/TTF/arial.ttf", 16);
	app->small_font = font;

	return 0;
}

int meh_app_destroy(App* app) {
	g_assert(app != NULL);

	/* Free the resource */
	meh_font_destroy(app->small_font);
	app->small_font = NULL;
	meh_window_destroy(app->window);
	app->window = NULL;

	return 0;
}

