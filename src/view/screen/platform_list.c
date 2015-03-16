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
#include "view/screen/fade.h"
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
	SDL_Color white = { 255, 255, 255, 255 };
	data->title = meh_widget_text_new(app->big_font, "mehstation 1.0", 50, 50, 300, 50, white, FALSE);
	data->title->x = meh_transition_start(MEH_TRANSITION_CUBIC, -200, 50, 1000);
	meh_screen_add_text_transitions(screen, data->title);

	/* No platforms text */
	data->no_platforms_widget = meh_widget_text_new(app->big_font, "No platforms configured", 100, 100, 200, 40, white, FALSE);

	/* Platforms */
	data->icons_widgets = g_queue_new();
	data->platforms_icons = g_queue_new();

	/* Load the data / icons / widgets of every platforms */
	for (int i = 0; i < g_queue_get_length(data->platforms); i++) {
		Platform* platform = g_queue_peek_nth(data->platforms, i);

		/* load the platform icon */
		SDL_Texture* p_texture = NULL;
		if (platform->icon == NULL || strlen(platform->icon) == 0) {
			/* create a texture with just the text of the platform */
			p_texture = meh_font_render_on_texture(
					app->window->sdl_renderer,
					app->small_font,
					platform->name,
					white,
					TRUE);
		} else {
			/* load the icon */
			p_texture = meh_image_load_file(app->window->sdl_renderer, platform->icon);
		}

		if (p_texture == NULL) {
			g_critical("Can't load the icon of the platform %s" ,platform->name);
		}

		g_queue_push_tail(data->platforms_icons, p_texture);

		/* create the platform widget */
		WidgetImage* platform_widget = meh_widget_image_new(p_texture, 100, 100 + (i*160), 150, 150);
		g_queue_push_tail(data->icons_widgets, platform_widget);
	}

	/* Selection */
	data->selection_widget = meh_widget_rect_new(95, 95, 160, 160, white, FALSE);

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
		/* free platforms widget */
		for (int i = 0; i < g_queue_get_length(data->icons_widgets); i++) {
			WidgetImage* widget = g_queue_peek_nth(data->icons_widgets, i);
			meh_widget_image_destroy(widget);
		}
		g_queue_free(data->platforms_icons);

		/* free platforms icons texture */
		for (int i = 0; i < g_queue_get_length(data->platforms_icons); i++) {
			SDL_Texture* text = g_queue_peek_nth(data->platforms_icons, i);
			SDL_DestroyTexture(text);
		}
		g_queue_free(data->platforms_icons);

		/* free platform models */
		meh_model_platforms_destroy(data->platforms);

		/* various widgets */
		meh_widget_text_destroy(data->title);
		meh_widget_text_destroy(data->no_platforms_widget);

		meh_widget_rect_destroy(data->selection_widget);
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
				if (message->data == NULL) {
					meh_screen_platform_list_render(app, screen, TRUE);
				} else {
					gboolean* flip = (gboolean*)message->data;
					meh_screen_platform_list_render(app, screen, *flip);
				}
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
		Screen* fade_screen = meh_screen_fade_new(app, screen, exec_list_screen);
		meh_app_set_current_screen(app, fade_screen);
		/* NOTE we don't free the memory of the starting screen, the fade screen
		 * will do it. */
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
			data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, data->selection_widget->y.value, data->selected_platform*160 + 95, 100);
			meh_screen_add_rect_transitions(screen, data->selection_widget);
			break;
		case MEH_INPUT_BUTTON_DOWN:
			if (data->selected_platform == g_queue_get_length(meh_screen_platform_list_get_data(screen)->platforms)-1) {
				data->selected_platform = 0;
			} else {
				data->selected_platform += 1;
			}
			data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, data->selection_widget->y.value, data->selected_platform*160 + 95, 100);
			meh_screen_add_rect_transitions(screen, data->selection_widget);
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

int meh_screen_platform_list_render(App* app, Screen* screen, gboolean flip) {
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

	/* selection */
	meh_widget_rect_render(app->window, data->selection_widget);

	/* icon */
	for (int i = 0; i < g_queue_get_length(data->icons_widgets); i++) {
		WidgetImage* widget = g_queue_peek_nth(data->icons_widgets, i);
		meh_widget_image_render(app->window, widget);
	}
	
	if (flip == TRUE) {
		meh_window_render(app->window);
	}

	return 0;
}
