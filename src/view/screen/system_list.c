#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/message.h"
#include "view/screen.h"
#include "view/screen/system_list.h"

Screen* meh_screen_system_list_new() {
	Screen* screen = meh_screen_new();

	screen->name = g_strdup("System list screen");
	screen->messages_handler = &meh_screen_system_list_messages_handler;

	return screen;
}

int meh_screen_system_list_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				int* pressed_button = (int*)message->data;
				meh_screen_system_list_button_pressed(app, screen, *pressed_button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				int* delta_time = (int*)message->data;
				meh_screen_system_list_update(screen, *delta_time);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_screen_system_list_render(app, screen);
			}
			break;
	}

	return 0;
}

/*
 * meh_screen_system_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_system_list_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	switch (pressed_button) {
		/* Return to the front page */
		case MEH_INPUT_SPECIAL_ESCAPE:
			/* Switch the current_screen to the parent screen if any */
			if (screen->parent_screen != NULL) {
				meh_app_set_current_screen(app, screen->parent_screen);
				/* this one won't be used anymore. */
				g_free(screen);
			} else {
				app->mainloop.running = FALSE;
			}
			break;
	}
}

int meh_screen_system_list_update(Screen* screen, int delta_time) {
	g_assert(screen != NULL);

	return 0;
}

int meh_screen_system_list_render(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	SDL_Color white = { 255, 255, 255 };
	meh_window_render_text(app->window, app->small_font, "mehstation 1.0", white, 50, 50);
	meh_window_render_text(app->window, app->small_font, "mehstation 1.0", white, 100, 100);
	
	meh_window_render(app->window);
	SDL_Delay(10); /* TODO delta */
	return 0;
}
