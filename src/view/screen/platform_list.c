/*
 * mehstation - Platform list screen.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include <glib.h>

#include "system/app.h"
#include "system/consts.h"
#include "system/input.h"
#include "system/message.h"
#include "system/transition.h"
#include "system/db/models.h"
#include "view/screen.h"
#include "view/widget_text.h"
#include "view/screen/executable_list.h"
#include "view/screen/platform_list.h"

Screen* meh_screen_platform_list_new(App* app) {
	Screen* screen = meh_screen_new();

	screen->name = g_strdup("Platform list screen");
	screen->messages_handler = &meh_screen_platform_list_messages_handler;
	screen->destroy_data = &meh_screen_platform_list_destroy_data;

	/* init the custom data. */
	PlatformListData* data = g_new(PlatformListData, 1);	
	data->platforms = meh_db_get_platforms(app->db);
	data->selected_platform = 0;

	/*
	 * Widgets
	 */

	/* Title */
	SDL_Color white = { 255, 255, 255 };
	data->title = meh_widget_text_new(app->big_font, "mehstation 1.0", 50, 50, white, FALSE);
	data->title->x = meh_transition_start(MEH_TRANSITION_CUBIC, -200, 50, 1000);
	meh_screen_add_text_transitions(screen, data->title);

	/* No platforms text */
	data->no_platforms_widget = meh_widget_text_new(app->big_font, "No platforms configured", 100, 100, white, FALSE);

	/* Platforms */
	data->platform_widgets = g_queue_new();
	for (int i = 0; i < g_queue_get_length(data->platforms); i++) {
		Platform* platform = g_queue_peek_nth(data->platforms, i);
		g_queue_push_tail(data->platform_widgets, meh_widget_text_new(app->small_font, platform->name, 100, 100 + (i*30), white, FALSE));
	}
	/* Selection */
	data->selection_widget = meh_widget_text_new(app->small_font, ">", 85, 100, white, FALSE);

	screen->data = data;

	return screen;
}

/*
 * meh_screen_platform_list_destroy_data role is to delete the typed data of the screen
 */
void meh_screen_platform_list_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	if (data != NULL) {
		/* Platform text widget. */
		for (int i = 0; i < g_queue_get_length(data->platform_widgets); i++) {
			WidgetText* widget = g_queue_peek_nth(data->platform_widgets, i);
			meh_widget_text_destroy(widget);
		}
		g_queue_free(data->platform_widgets);

		meh_model_platforms_destroy(data->platforms);

		meh_widget_text_destroy(data->title);
		meh_widget_text_destroy(data->no_platforms_widget);
		meh_widget_text_destroy(data->selection_widget);
	}
}

/*
 * meh_screen_platform_list_get_data returns the data of the platform_list screen
 */
PlatformListData* meh_screen_platform_list_get_data(Screen* screen) {
	PlatformListData* data = (PlatformListData*) screen->data;
	return data;
}

int meh_screen_platform_list_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}


	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				int* pressed_button = (int*)message->data;
				meh_screen_platform_list_button_pressed(app, screen, *pressed_button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_platform_list_update(screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_screen_platform_list_render(app, screen);
			}
			break;
	}

	return 0;
}

static void meh_screen_platform_list_start_platform(App* app, Screen* screen) {

	/* get the platform */
	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	Platform* platform = g_queue_peek_nth(data->platforms, data->selected_platform);

	if (platform != NULL) {
		/* create the child screen */
		Screen* exec_list_screen = meh_screen_exec_list_new(app, platform->id);
		exec_list_screen->parent_screen = screen;
		meh_app_set_current_screen(app, exec_list_screen);
	}
}

/*
 * meh_screen_platform_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_platform_list_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);

	switch (pressed_button) {
		case MEH_INPUT_SPECIAL_ESCAPE:
			app->mainloop.running = FALSE;
			break;
		case MEH_INPUT_BUTTON_A:
			meh_screen_platform_list_start_platform(app, screen);
			break;
		case MEH_INPUT_BUTTON_B:
			/* Switch the current_screen to the parent screen if any */
			if (screen->parent_screen != NULL) {
				meh_app_set_current_screen(app, screen->parent_screen);
				/* this one won't be used anymore. */
				meh_screen_destroy(screen);
			}
			break;
		case MEH_INPUT_BUTTON_UP:
			if (data->selected_platform == 0) {
				data->selected_platform = g_queue_get_length(meh_screen_platform_list_get_data(screen)->platforms)-1;
			} else {
				data->selected_platform -= 1;
			}
			data->selection_widget->y.value = data->selected_platform*30 + 100;
			break;
		case MEH_INPUT_BUTTON_DOWN:
			if (data->selected_platform == g_queue_get_length(meh_screen_platform_list_get_data(screen)->platforms)-1) {
				data->selected_platform = 0;
			} else {
				data->selected_platform += 1;
			}
			data->selection_widget->y.value = data->selected_platform*30 + 100;
			break;
	}
}

int meh_screen_platform_list_update(Screen* screen) {
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	g_assert(data != NULL);

	/* updates all the transition in the screen */
	meh_screen_update_transitions(screen);

	return 0;
}

int meh_screen_platform_list_render(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	g_assert(data != NULL);

	SDL_Color black = { 0, 0, 0 };
	int platform_count = g_queue_get_length(data->platforms);

	/* clear the screen */
	meh_window_clear(app->window, black);

	meh_widget_text_render(app->window, data->title);
	
	if (platform_count == 0) {
		meh_widget_text_render(app->window, data->no_platforms_widget);
	}

	for (int i = 0; i < g_queue_get_length(data->platform_widgets); i++) {
		WidgetText* widget = g_queue_peek_nth(data->platform_widgets, i);
		meh_widget_text_render(app->window, widget);
	}

	meh_widget_text_render(app->window, data->selection_widget);
	
	meh_window_render(app->window);
	return 0;
}
