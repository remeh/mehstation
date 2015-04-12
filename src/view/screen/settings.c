/*
 * mehstation - Settings screen..
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/consts.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/fade.h"
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

	SDL_Color white = { 255, 255, 255, 255 };
	data->title = meh_widget_text_new(app->big_font, "mehstation 1.0", 50, 50, 300, 50, white, FALSE);
	data->title_settings = meh_widget_text_new(app->big_font, "- settings", MEH_FAKE_WIDTH, 50, 180, 50, white, FALSE);
	data->title_settings->x = meh_transition_start(MEH_TRANSITION_CUBIC, MEH_FAKE_WIDTH+200, 330, 750);
	meh_screen_add_text_transitions(screen, data->title_settings);

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

	meh_widget_text_destroy(data->title);
	meh_widget_text_destroy(data->title_settings);

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
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_screen_settings_button_pressed(app, screen, data->button);
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

void meh_screen_settings_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SettingsData* data = meh_screen_settings_get_data(screen);

	switch (pressed_button) {
		case MEH_INPUT_BUTTON_B:
			if (screen->parent_screen != NULL) {
				/* make the - settings label go out */
				data->title_settings->x = meh_transition_start(MEH_TRANSITION_CUBIC, 330, MEH_FAKE_WIDTH+200, 300);
				meh_screen_add_text_transitions(screen, data->title_settings);

				/* back to the previous screen using a fade
				 * screen for the transition */
				Screen* fade_screen = meh_screen_fade_new(app, screen, screen->parent_screen);
				meh_app_set_current_screen(app, fade_screen, FALSE);
				/* NOTE we don't free the memory of the current screen, the fade screen
				 * will do it. */
			}
			break;
	}
}

void meh_screen_settings_render(App* app, Screen* screen, gboolean flip) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SettingsData* data = meh_screen_settings_get_data(screen);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	meh_widget_text_render(app->window, data->title);
	meh_widget_text_render(app->window, data->title_settings);

		meh_window_render(app->window);
}
