/*
 * mehstation - Configure input.
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
#include "view/screen/mapping.h"
#include "view/window.h"

Screen* meh_screen_mapping_new(App* app) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Gamepad configuration screen");
	screen->messages_handler = &meh_screen_mapping_messages_handler;
	screen->destroy_data = &meh_screen_mapping_destroy_data;

	/*
	 * Custom data
	 */
	MappingData* data = g_new(MappingData, 1);

	screen->data = data;

	return screen;
}

int meh_screen_mapping_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				int* pressed_button = (int*)message->data;
				meh_screen_mapping_button_pressed(app, screen, *pressed_button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_mapping_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			if (message->data == NULL) {
				meh_screen_mapping_render(app, screen, TRUE);
			} else {
				gboolean* flip = (gboolean*)message->data;
				meh_screen_mapping_render(app, screen, *flip);
			}
			break;
	}

	return 0;
}

void meh_screen_mapping_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	MappingData* data = meh_screen_mapping_get_data(screen);

	screen->data = NULL;
}

/*
 * meh_screen_mapping_get_data returns the specific data of the
 * screen
 */
MappingData* meh_screen_mapping_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (MappingData*) screen->data;
}

/*
 * meh_screen_mapping_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_mapping_button_pressed(App* app, Screen* screen, int pressed_button) {
	switch (pressed_button) {
		/* Escape on the starting screen quit the app */
		case MEH_INPUT_SPECIAL_ESCAPE:
			/* TODO close the current screen */
			break;
		case MEH_INPUT_BUTTON_START:
		case MEH_INPUT_BUTTON_B:
		case MEH_INPUT_BUTTON_A:
		case MEH_INPUT_BUTTON_L:
		case MEH_INPUT_BUTTON_R:
			/* TODO */
			break;
	}
}

/*
 * meh_screen_mapping_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_mapping_update(App* app, Screen* screen) {
	/* update the transition of the current screen */
	meh_screen_update_transitions(screen);

	/* TODO */

	return 0;
}

/*
 * meh_screen_mapping_render is the rendering of the starting screen.
 */
void meh_screen_mapping_render(App* app, Screen* screen, gboolean flip) {
	g_assert(screen != NULL);
	g_assert(app != NULL);

	MappingData* data = meh_screen_mapping_get_data(screen);
	g_assert(data != NULL);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	/* if any, render the parent screen */
	/* TODO */

	/* render the overlay */
	meh_widget_rect_render(app->window, data->overlay);

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
}
