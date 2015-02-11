#include "SDL2/SDL_image.h"
#include "glib-2.0/glib.h"

#include "view/image.h"
#include "view/screen.h"
#include "view/screen/starting.h"
#include "system/app.h"
#include "system/input.h"
#include "system/settings.h"

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

	if ( !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ) {
		g_critical("SDL_image can't initialize: %s\n", TTF_GetError());
		return 1;
	}

	g_message("SDL init OK.");

	meh_settings_print_system_infos();

	/* Read the settings */
	Settings settings;
	meh_settings_read(&settings, "mehstation.conf");
	app->settings = settings;

	/* Open the main window */
	Window* window = meh_window_create(settings.width, settings.height, settings.fullscreen);
	app->window = window;

	/* Opens some font. */
	Font* font = meh_font_open("res/fonts/FreeMonoBold.ttf", 16);
	app->small_font = font;

	/* Input manager */
	InputManager* input_manager = meh_input_manager_new();
	app->input_manager = input_manager;

	/* Sets the starting screen as the current screen */
	Screen* starting_screen = meh_screen_starting_new();
	if (starting_screen == NULL) {
		g_critical("Can't init the starting screen.");
		return 1;
	} 

	meh_app_set_current_screen(app, starting_screen);

	return 0;
}

/*
 * meh_app_destroy frees the resources of the given app
 * and closes every system (SDL, ...)
 */
int meh_app_destroy(App* app) {
	g_assert(app != NULL);

	/* Free the resource */
	meh_font_destroy(app->small_font);
	app->small_font = NULL;
	meh_window_destroy(app->window);
	app->window = NULL;

	SDL_Quit();
	TTF_Quit();

	return 0;
}

void meh_app_set_current_screen(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	g_message("Setting the current screen to : %s", screen->name);
	app->current_screen = screen;
}

/*
 * meh_app_main_loop is the main loop running for the whole time 
 * of the process.
 */
int meh_app_main_loop(App* app) {
	g_assert(app != NULL);

	//int last_time = SDL_GetTicks();
	app->mainloop.running = TRUE;

	/* application lifecycle */
	while (app->mainloop.running) {
		meh_app_main_loop_event(app);
		meh_app_main_loop_update(app);
		meh_app_main_loop_render(app);
	}

	return 0;
}

/*
 * meh_app_main_loop_event is the event handling part
 * of the pipeline.
 */
void meh_app_main_loop_event(App* app) {
	g_assert(app != NULL);

	SDL_Event* event = &(app->mainloop.event);
	while (SDL_PollEvent(event)) {
		switch (event->type) {
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				meh_input_manager_keyboard_read_event(app->input_manager, event);
				break;
			case SDL_QUIT:
				app->mainloop.running = FALSE;
				break;
		}
	}

	/* Generate events from input */
	GSList* list_events = meh_input_manager_generate_events(app->input_manager);

	int i = 0;
	for (i = 0; i < g_slist_length(list_events); i++) {
		/* create the Event */
		int* event_id = g_slist_nth_data(list_events, i);
		Event* event = g_new(Event, 1);
		event->id = *event_id;
		/* send it to the current screen */
		meh_app_send_event(app, event);
	}

	g_slist_free(list_events);
}


/*
 * meh_app_main_loop_render is the rendering part of the pipeline.
 */
void meh_app_main_loop_render(App* app) {
	g_assert(app != NULL);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	SDL_Texture* texture = meh_image_load_file(app->window->sdl_renderer, "./image.png");
	SDL_Rect rect = { 0, 0, 500, 500 };
	meh_window_render_texture(app->window, texture, rect);
	SDL_DestroyTexture(texture);

	meh_window_render_text(app->window, app->small_font, "mehstation 1.0", black, 50, 50);

	meh_window_render(app->window);
	SDL_Delay(10); /* TODO delta */
}

void meh_app_main_loop_update(App* app) {
	g_assert(app != NULL);

	if (app->current_screen != NULL) {
		/* TODO delta_time */
		app->current_screen->update(app->current_screen, 0);
	}
}

void meh_app_send_event(App* app, Event* event) {
	g_assert(app != NULL);

	if (event == NULL) {
		g_warning("NULL event has been seen in meh_app_send_event");
		return;
	}

	/* route the event to the screen. */
	app->current_screen->events_handler(app->current_screen, event);
}
