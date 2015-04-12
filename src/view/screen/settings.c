/*
 * mehstation - Settings screen..
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/consts.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/settings.h"

Screen* meh_screen_settings_new(App* app) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Settings screen");
	screen->messages_handler = &meh_screen_settings_messages_handler;
	screen->destroy_data = &meh_screen_settings_destroy_data;

	/*
	 * Custom data
	 */
	SettingsData* data = g_new(SettingsData, 1);
	screen->data = data;

	return screen;
}

SettingsData* meh_screen_settings_get_data(Screen* screen) {
	g_assert(screen != NULL);

	if (screen->data == NULL) {
		return screen->data;
	}

	return (SettingsData*) screen->data;
}

/*
 * meh_screen_settings_destroy_data destroys the additional data
 * of the settings screen.
 */
void meh_screen_settings_destroy_data(Screen* screen) {
	SettingsData* data = meh_screen_settings_get_data(screen);
	screen->data = NULL;
}

int meh_screen_settings_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_UPDATE:
			{
				meh_screen_settings_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			if (message->data == NULL) {
				meh_screen_settings_render(app, screen, TRUE);
			} else {
				gboolean* flip = (gboolean*)message->data;
				meh_screen_settings_render(app, screen, *flip);
			}
			break;
	}

	return 0;
}

int meh_screen_settings_update(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	meh_screen_update_transitions(screen);

	return 0;
}

void meh_screen_settings_render(App* app, Screen* screen, gboolean flip) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
}


