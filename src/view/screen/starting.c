/*
 * mehstation - Splashscreen.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/message.h"
#include "view/image.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/fade.h"
#include "view/screen/starting.h"
#include "view/screen/mapping.h"
#include "view/screen/platform_list.h"
#include "view/window.h"

Screen* meh_screen_starting_new(App* app) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Starting screen");
	screen->messages_handler = &meh_screen_starting_messages_handler;
	screen->destroy_data = &meh_screen_starting_destroy_data;

	/*
	 * Custom data
	 */
	StartingData* data = g_new(StartingData, 1);

	/* Splashscreen */
	data->splash_texture = meh_image_load_file(app->window->sdl_renderer, "res/splashscreen.png");
	data->splash = meh_widget_image_new(data->splash_texture, 0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT);
	data->done = FALSE;

	screen->data = data;

	return screen;
}

int meh_screen_starting_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_screen_starting_button_pressed(app, screen, data->button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_starting_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			if (message->data == NULL) {
				meh_screen_starting_render(app, screen, TRUE);
			} else {
				gboolean* flip = (gboolean*)message->data;
				meh_screen_starting_render(app, screen, *flip);
			}
			break;
	}

	return 0;
}

void meh_screen_starting_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	StartingData* data = meh_screen_starting_get_data(screen);
	if (data != NULL && data->splash_texture != NULL) {
		SDL_DestroyTexture(data->splash_texture);
	}
	screen->data = NULL;
}

StartingData* meh_screen_starting_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (StartingData*) screen->data;
}

static void meh_screen_starting_go_to_next_screen(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	/* create and switch to the platform list screen
	 * if we don't need to configure a mapping */
	if (meh_input_manager_has_something_plugged(app->input_manager) && !app->flags.configure_mapping) {
		Screen* platform_list_screen = meh_screen_platform_list_new(app);
		Screen* fade_screen = meh_screen_fade_new(app, screen, platform_list_screen);
		meh_app_set_current_screen(app, fade_screen);
	} else {
		Screen* mapping_screen = meh_screen_mapping_new(app);
		Screen* fade_screen = meh_screen_fade_new(app, screen, mapping_screen);
		meh_app_set_current_screen(app, fade_screen);
	}
	/* NOTE we don't free the memory of the starting screen, the fade screen
	 * will do it. */
}

/*
 * meh_screen_starting_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_starting_button_pressed(App* app, Screen* screen, int pressed_button) {
	switch (pressed_button) {
		/* Escape on the starting screen quit the app */
		case MEH_INPUT_SPECIAL_ESCAPE:
			app->mainloop.running = FALSE;
			break;
		default:
			meh_screen_starting_go_to_next_screen(app, screen);
			break;
	}
}

/*
 * meh_screen_starting_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_starting_update(App* app, Screen* screen) {
	/* Animate the splashscreen */
	meh_screen_update_transitions(screen);

	StartingData* data = meh_screen_starting_get_data(screen);
	g_assert(data != NULL);

	/*
	 * Wait 5s before going to the next screen.
	 */
	if (SDL_GetTicks() > 5000 && data->done == FALSE) {
		meh_screen_starting_go_to_next_screen(app, screen);
		data->done = TRUE; /* don't redo it while the fading screen is refreshing */
	}

	return 0;
}

/*
 * meh_screen_starting_render is the rendering of the starting screen.
 */
void meh_screen_starting_render(App* app, Screen* screen, gboolean flip) {
	g_assert(screen != NULL);
	g_assert(app != NULL);

	StartingData* data = meh_screen_starting_get_data(screen);
	g_assert(data != NULL);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	/* draw the splashscreen */
	meh_widget_image_render(app->window, data->splash);

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
}
