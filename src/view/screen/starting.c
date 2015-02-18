#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/message.h"
#include "view/image.h"
#include "view/screen.h"
#include "view/screen/starting.h"
#include "view/screen/platform_list.h"
#include "view/window.h"

Screen* meh_screen_starting_new() {
	Screen* screen = meh_screen_new();

	screen->name = g_strdup("Starting screen");
	screen->messages_handler = &meh_screen_starting_messages_handler;

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
			meh_screen_starting_go_to_platform_list(app, screen);
			break;
	}
}

/*
 * meh_screen_starting_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_starting_update(App* app, Screen* screen, int delta_time) {

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
