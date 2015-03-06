#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/message.h"
#include "view/image.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/starting.h"
#include "view/screen/platform_list.h"
#include "view/window.h"

Screen* meh_screen_starting_new(App* app) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new();

	screen->name = g_strdup("Starting screen");
	screen->messages_handler = &meh_screen_starting_messages_handler;
	screen->destroy_data = &meh_screen_starting_destroy_data;

	/*
	 * Custom data
	 */
	StartingData* data = g_new(StartingData, 1);

	/* Splashscreen */
	data->splash_texture = meh_image_load_file(app->window->sdl_renderer, "res/splashscreen.png");
	data->splash = meh_widget_image_new(data->splash_texture, 0.0f, 0.0f, 1.0f, 1.0f);

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
				int* pressed_button = (int*)message->data;
				meh_screen_starting_button_pressed(app, screen, *pressed_button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				if (message->data == NULL) {
					g_warning("No data with a MEH_MSG_UPDATE in starting screen.");
					return 2;
				}
				int* delta_time = (int*)message->data;
				meh_screen_starting_update(app, screen, *delta_time);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_screen_starting_render(app, screen);
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

static void meh_screen_starting_go_to_platform_list(App* app, Screen* screen) {
	/* create and switch  to the system list screen. */
	Screen* platform_list_screen = meh_screen_platform_list_new(app);
	meh_app_set_current_screen(app, platform_list_screen);
	/* free the memory of the starting screen */
	meh_screen_destroy(screen);
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
		case MEH_INPUT_BUTTON_START:
		case MEH_INPUT_BUTTON_B:
		case MEH_INPUT_BUTTON_A:
		case MEH_INPUT_BUTTON_L:
		case MEH_INPUT_BUTTON_R:
			meh_screen_starting_go_to_platform_list(app, screen);
			break;
	}
}

/*
 * meh_screen_starting_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_starting_update(App* app, Screen* screen, int delta_time) {
	/* Animate the splashscreen */
	meh_screen_update_transitions(screen);

	/*
	 * Wait 5s before going to the system list selection.
	 */
	if (SDL_GetTicks() > 5000) {
		meh_screen_starting_go_to_platform_list(app, screen);
	}

	return 0;
}

/*
 * meh_screen_starting_render is the rendering of the starting screen.
 */
void meh_screen_starting_render(App* app, Screen* screen) {
	g_assert(screen != NULL);
	g_assert(app != NULL);

	StartingData* data = meh_screen_starting_get_data(screen);
	g_assert(data != NULL);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	/* draw the splashscreen */
	meh_widget_image_render(app->window, data->splash);

	meh_window_render(app->window);
}
