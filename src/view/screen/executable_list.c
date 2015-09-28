/*
 * mehstation - Screen of executables.
 *
 * Copyright © 2015 Rémy Mathieu
 *
 * TODO We could free the cache when an executable is launched.
 */

#include <glib.h>

#include "system/app.h"
#include "system/consts.h"
#include "system/db.h"
#include "system/input.h"
#include "system/message.h"
#include "system/transition.h"
#include "system/db/models.h"
#include "view/image.h"
#include "view/widget_text.h"
#include "view/screen.h"
#include "view/screen/fade.h"
#include "view/screen/executable_list.h"
#include "view/screen/launch.h"
#include "view/screen/popup.h"

static void meh_exec_create_widgets(App* app, Screen* screen, ExecutableListData* data);
static void meh_exec_list_destroy_resources(Screen* screen);
static void meh_exec_list_load_resources(App* app, Screen* screen);
static void meh_exec_list_start_executable(App* app, Screen* screen);
static void meh_exec_list_select_resources(Screen* screen);
static void meh_exec_list_start_bg_anim(Screen* screen);
static void meh_exec_list_resolve_tex(Screen* screen);

Screen* meh_exec_list_new(App* app, int platform_id) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Executable list screen");
	screen->messages_handler = &meh_exec_list_messages_handler;
	screen->destroy_data = &meh_exec_list_destroy_data;

	/*
	 * Init the custom data.
	 */

	ExecutableListData* data = g_new(ExecutableListData, 1);	

	/* get the platform */
	data->platform = meh_db_get_platform(app->db, platform_id);
	g_assert(data->platform != NULL);

	/* get the executables */
	data->executables = meh_db_get_platform_executables(app->db, data->platform, TRUE);
	data->executables_length = g_queue_get_length(data->executables);
	data->cache_executables_id = g_queue_new();
	data->selected_executable = 0;

	/* display resources */
	data->textures = NULL;
	data->background = -1;
	data->cover = -1;
	data->logo = -1;
	data->screenshots[0] = data->screenshots[1] = data->screenshots[2] = -1;

	data->executable_widgets = g_queue_new();

	data->exec_list_video = NULL;

	/* create widgets */
	meh_exec_create_widgets(app, screen, data);

	screen->data = data;

	/*
	 * Select and load the first resources
	 * then refresh all the text textures by faking
	 * a cursor movement.
	 */
	meh_exec_list_after_cursor_move(app, screen, -1);

	return screen;
}

static void meh_exec_list_metadata_init(App* app, Screen* screen,
										WidgetText** label, WidgetText** value, gchar* title, int delay,
										int title_x, int title_y, int title_w, int title_h,
										int value_x, int value_y, int value_w, int value_h) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SDL_Color white = { 255, 255, 255, 255 };
	*label = meh_widget_text_new(app->small_bold_font, title, title_x, title_y, title_w, title_h, white, TRUE);
	*value = meh_widget_text_new(app->small_font, NULL, value_x, value_y, value_w, value_h, white, TRUE);
	(*value)->x = meh_transition_start(MEH_TRANSITION_CUBIC, value_x+MEH_FAKE_WIDTH, value_x, 300+delay);
	meh_screen_add_text_transitions(screen, (*value));
}

static void meh_exec_create_widgets(App* app, Screen* screen, ExecutableListData* data) {
	g_assert(app != NULL);
	g_assert(screen != NULL);
	g_assert(data != NULL);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color white_transparent = { 255, 255, 255, 50 };
	SDL_Color gray = { 10, 10, 10, 235 };

	/* Selection. */
	data->selection_widget = meh_widget_rect_new(0, -100, 415, 28, white_transparent, TRUE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 130, 500);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/* Background */
	data->background_widget = meh_widget_image_new(NULL, -50, -50, MEH_FAKE_WIDTH+50, MEH_FAKE_HEIGHT+50);

	/* Background hover */
	data->bg_hover_widget = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, gray, TRUE); 

	/* Header */
	data->header_text_widget = meh_widget_text_new(app->big_font, data->platform->name, 20, 55, 400, 40, white, TRUE);
	data->header_text_widget->uppercase = TRUE;
	data->header_text_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, -200, 20, 300);
	meh_screen_add_text_transitions(screen, data->header_text_widget);

	/* Executables */
	for (int i = 0; i < MEH_EXEC_LIST_SIZE; i++) {
		WidgetText* text = meh_widget_text_new(app->small_font, "", 55, 130+(i*32), 350, 30, white, FALSE);
		text->uppercase = TRUE; /* executables name in uppercase */
		g_queue_push_tail(data->executable_widgets, text);
	}

	/*
	 * Extra information
	 */

	/* Genres */
	meh_exec_list_metadata_init(app, screen,
								&data->genres_l_widget, &data->genres_widget,
								"Genres", 0,
								870, 396, 80, 30,
								1070, 400, 150, 30);

	/* Players */
	meh_exec_list_metadata_init(app, screen,
								&data->players_l_widget, &data->players_widget,
								"Players", 0,
								870, 426, 80, 30,
								1070, 430, 150, 30);

	/* Publisher */
	meh_exec_list_metadata_init(app, screen,
								&data->publisher_l_widget, &data->publisher_widget,
								"Publisher", 150,
								870, 456, 120, 30,
								1070, 460, 150, 30);

	/* Developer */
	meh_exec_list_metadata_init(app, screen,
								&data->developer_l_widget, &data->developer_widget,
								"Developer", 150,
								870, 488, 150, 30,
								1070, 490, 150, 30);

	/* Rating */
	meh_exec_list_metadata_init(app, screen,
								&data->rating_l_widget, &data->rating_widget,
								"Rating", 300,
								870, 518, 100, 30,
								1070, 520, 150, 30);

	/* Release date */
	meh_exec_list_metadata_init(app, screen,
								&data->release_date_l_widget, &data->release_date_widget,
								 "Release date", 300,
								 870, 548, 150, 30,
								 1070, 550, 150, 30);

	/* Cover */
	data->cover_widget = meh_widget_image_new(NULL, 1030, 60, 200, 300);

	/* Logo */
	data->logo_widget = meh_widget_image_new(NULL, 530, 60, 350, 100);

	/* Screenshots */
	for (int i = 0; i < 3; i++) {
		data->screenshots_widget[i] = meh_widget_image_new(NULL, 500 + (265*i), 620, 190, 80);
	}

	/* Description */
	data->description_widget = meh_widget_text_new(app->small_font, NULL, 500, 180, 450, 280, white, FALSE);
	data->description_widget->multi = TRUE;
}

/*
 * meh_exec_list_start_bg_anim slighty moves the background to give
 * an impression of dynamic.
 */
static void meh_exec_list_start_bg_anim(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);
	g_assert(data != NULL);

	// TODO(remy): add some random direction here.
	data->background_widget->x = meh_transition_start(MEH_TRANSITION_LINEAR, -50, 0, 10000);
	data->background_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, -50, 0, 10000);
	meh_screen_add_image_transitions(screen, data->background_widget);
}

/*
 * meh_exec_list_destroy_data role is to delete the typed data of the screen
 */
void meh_exec_list_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);
	if (data != NULL) {
		meh_model_platform_destroy(data->platform);
		meh_model_executables_destroy(data->executables);

		/* Destroy the widgets */
		meh_widget_image_destroy(data->background_widget);
		meh_widget_rect_destroy(data->selection_widget);
		meh_widget_rect_destroy(data->bg_hover_widget);
		meh_widget_image_destroy(data->cover_widget);
		meh_widget_image_destroy(data->logo_widget);
		for (int i = 0; i < 3; i++) {
			meh_widget_image_destroy(data->screenshots_widget[i]);
		}
		meh_widget_text_destroy(data->header_text_widget);

		meh_widget_text_destroy(data->genres_l_widget);
		meh_widget_text_destroy(data->genres_widget);
		meh_widget_text_destroy(data->players_l_widget);
		meh_widget_text_destroy(data->players_widget);
		meh_widget_text_destroy(data->publisher_l_widget);
		meh_widget_text_destroy(data->publisher_widget);
		meh_widget_text_destroy(data->developer_l_widget);
		meh_widget_text_destroy(data->developer_widget);
		meh_widget_text_destroy(data->rating_l_widget);
		meh_widget_text_destroy(data->rating_widget);
		meh_widget_text_destroy(data->release_date_l_widget);
		meh_widget_text_destroy(data->release_date_widget);

		meh_widget_text_destroy(data->description_widget);

		for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
			meh_widget_text_destroy( g_queue_peek_nth( data->executable_widgets, i) );
		}
		g_queue_free(data->executable_widgets);

		/* Free the executables id cache. */
		for (unsigned int i = 0; i < g_queue_get_length(data->cache_executables_id); i++) {
			g_free(g_queue_peek_nth(data->cache_executables_id, i));
		}
		g_queue_free(data->cache_executables_id);

		/* destroy the video overlay */
		meh_exec_list_video_destroy(data->exec_list_video);
		data->exec_list_video = NULL;

		/* we must free the textures cache */
		meh_exec_list_destroy_resources(screen);
	}
}

/*
 * meh_exec_list_destroy_resources takes care of destroying the
 * resources.
 */
static void meh_exec_list_destroy_resources(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	if (data->textures == NULL) {
		return;
	}

	GList* keys = g_hash_table_get_keys(data->textures);
	for (unsigned int i = 0; i < g_list_length(keys); i++) {
		int* key = g_list_nth_data(keys, i);
		SDL_Texture* texture = g_hash_table_lookup(data->textures, key);
		if (texture != NULL) {
			g_debug("Freeing the texture id %d", *key);
			SDL_DestroyTexture(texture);
		}
	}

	g_list_free(keys);
	g_hash_table_destroy(data->textures);
}

/*
 * meh_exec_list_is_in_delta checks whether the given index is in the delta
 * of the current selection.
 *
 * Ex:
 *
 * 1            With a delta of 3, the values 9, 1, 2, 4, 5, 6 are included
 * 2            in the delta.
 * 3 <--
 * 4            This method has been created for cache cleaning.
 * 5
 * 6
 * 7
 * 8
 * 9
 */
static gboolean meh_exec_list_is_in_delta(ExecutableListData* data, int idx) {
	int top_limit = idx - MEH_EXEC_LIST_DELTA;
	int bottom_limit = idx + MEH_EXEC_LIST_DELTA;

	/* Special case, if we outbound on low and high, we are included. */
	if (bottom_limit > data->executables_length && top_limit < 0) {
		return TRUE;
	}

	int cursor = data->selected_executable;
	int j = 0;
	for (j = 0; j < MEH_EXEC_LIST_DELTA+1; j++) {
		if (cursor > data->executables_length) {
			cursor = 0;
		}
		if (idx == cursor) {
			return TRUE;
		}
		cursor++;
	}

	cursor = data->selected_executable;
	j = 0;
	for (j = 0; j < MEH_EXEC_LIST_DELTA+1; j++) {
		if (cursor < 0) {
			cursor = data->executables_length-1;
		}
		if (idx == cursor) {
			return TRUE;
		}
		cursor--;
	}

	return FALSE;
}

/*
 * meh_exec_list_delete_some_cache looks for which cache we could
 * free without impacting the user experience.
 */
static void meh_exec_list_delete_some_cache(Screen* screen) {
	g_assert(screen != NULL);	

	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* Get the current executable */
	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (current_executable == NULL || current_executable->resources == NULL) {
		return;
	}

	int watchdog = 20; /* Avoid an infinite loop. */
	while (g_queue_get_length(data->cache_executables_id) > MEH_EXEC_LIST_MAX_CACHE && watchdog > 0) {
		int* idx = g_queue_pop_head(data->cache_executables_id);
		if (*idx != current_executable->id &&
			!meh_exec_list_is_in_delta(data, *idx)) { /* do not free the resources of the current selection */
			/* executable for which we want to free the resources */
			Executable* exec_to_clear_for = g_queue_peek_nth(data->executables, *idx);
			if (exec_to_clear_for != NULL && exec_to_clear_for->resources != NULL) {
				g_debug("Cache cleaning of the resources of %s", exec_to_clear_for->display_name);
				/* free the resources of this executable */
				for (unsigned int i = 0; i < g_queue_get_length(exec_to_clear_for->resources); i++) {
					ExecutableResource* resource = g_queue_peek_nth(exec_to_clear_for->resources, i);
					if (resource == NULL) {
						continue;
					}

					/* free the associated texture */
					SDL_Texture* texture = g_hash_table_lookup(data->textures, &(resource->id));
					if (texture != NULL) { /* can be null because we don't load all the resources */
						SDL_DestroyTexture(texture);
						g_hash_table_remove(data->textures, &(resource->id));
						g_debug("Cache clean of %s ID %d", resource->type, resource->id);
					}
				}
			}
			/* finally free the data of the entry in the cache */
			g_free(idx);
		} else {
			/* we can't delete it, it's the current selection
			 * re-add it into the cache list. */
			g_queue_push_tail(data->cache_executables_id, idx);
		}
		watchdog--;
	}
}

/*
 * meh_exec_list_select_resources uses the resources of the currently selected
 * executable to select a background and a cover.
 */
static void meh_exec_list_select_resources(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	Executable* executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (executable == NULL || executable->resources == NULL) {
		return;
	}

	/*
	 * Start by re-init the screenshots and others indexes
	 */

	data->cover = -1;
	data->logo = -1;
	data->screenshots[0] = data->screenshots[1] = data->screenshots[2] = -1;
	data->cover_widget->texture = data->logo_widget->texture = NULL;
	data->screenshots_widget[0]->texture = data->screenshots_widget[1]->texture = data->screenshots_widget[2]->texture = NULL;

	/*
	 * Select a random resource if any and tries to not take a cover nor a logo if possible.
	 */
	int length = g_queue_get_length(executable->resources);
	if (length == 0) {
		return;
	}

	int watchdog = 5; /* don't do too many tries. */
	ExecutableResource* resource = NULL;
	while (watchdog > 0) {
		int rand = g_random_int_range(0, length);

		resource = g_queue_peek_nth(executable->resources, rand);
		if (resource == NULL) {
			return;
		}

		if (g_strcmp0(resource->type, "video") != 0 &&
				g_strcmp0(resource->type, "cover") != 0 && g_strcmp0(resource->type, "logo") != 0) {
			/* We found something that's not a cover, nor a video and nor a logo, perfect. */
			break;
		} else {
			if (g_queue_get_length(executable->resources) == 1) {
				/* We only found a cover or a logo, but we have only one resource, so we can also stop here. */
				break;
			}
		}

		watchdog--;
	}

	if (resource != NULL) {
		data->background = resource->id;
		g_debug("Selected background : %d.", resource->id);
	}

	/*
	 * Select a cover and some screenshots.
	 */

	int found_screenshots = 0;
	for (unsigned int i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* res = g_queue_peek_nth(executable->resources, i);
		if (res != NULL) {
			if (g_strcmp0(res->type, "cover") == 0) {
				data->cover = res->id;
				g_debug("Selected cover: %d", res->id);
			} else if (g_strcmp0(res->type, "logo") == 0) {
				data->logo = res->id;
				g_debug("Selected logo: %d", res->id);
			} else if ((g_strcmp0(res->type, "screenshot") == 0 ||  /* TODO Select random ones */
					   g_strcmp0(res->type, "fanart") == 0) &&
					   found_screenshots < 3) {
				data->screenshots[found_screenshots] = res->id;
				g_debug("Selected %d screenshot/fanart: %d", found_screenshots, res->id);
				found_screenshots++;
			}
		}
	}
}

/*
 * meh_exec_list_get_data returns the data of the executable_list screen
 */
ExecutableListData* meh_exec_list_get_data(Screen* screen) {
	ExecutableListData* data = (ExecutableListData*) screen->data;
	g_assert(data != NULL);
	return data;
}

int meh_exec_list_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_exec_list_button_pressed(app, screen, data->button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_exec_list_update(screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				if (message->data == NULL) {
					meh_exec_list_render(app, screen, TRUE);
				} else {
					gboolean* flip = (gboolean*)message->data;
					meh_exec_list_render(app, screen, *flip);
				}
			}
			break;
	}

	return 0;
}

/*
 * meh_exec_list_load_resources loads the resources of the currently
 * selected game.
 */
static void meh_exec_list_load_resources(App* app, Screen* screen) {
	g_assert(app != NULL);	
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	if (data->executables == NULL || data->executables_length == 0) {
		return;
	}
	
	Executable* executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (executable == NULL || executable->resources == NULL) {
		return;
	}

	/* Create the hash table if not existing */
	if (data->textures == NULL) {
		data->textures = g_hash_table_new(g_int_hash, g_int_equal);
	}

	/* Loads the textures described in the executable resources
	 * if it's not already in the cache */
	for (unsigned int i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* resource = g_queue_peek_nth(executable->resources, i);
		if (resource == NULL) {
			continue;
		}

		/* ensure that it's an image */
		if (g_strcmp0(resource->type, "video") == 0) {
			continue;
		}

		/* Load only the needed resources. */
		int rid = resource->id;
		if (rid != data->background && rid != data->cover && rid != data->logo &&
			rid != data->screenshots[0] && rid != data->screenshots[1] &&
			rid != data->screenshots[2]) {
			continue;
		}

		/* Look whether or not it's already in the cache. */
		if (g_hash_table_lookup(data->textures, &(resource->id)) != NULL) {
			g_debug("Not reloading the %s ID %d", resource->type, resource->id);
			continue;
		}

		g_debug("Loading the %s ID %d", resource->type, resource->id);
		SDL_Texture* texture = meh_image_load_file(app->window->sdl_renderer, resource->filepath);
		if (texture != NULL) {
			int* id = g_new(int, 1); *id = resource->id;
			g_hash_table_insert(data->textures, id, texture);
		}
	}

	/* Add to the cache the information that we've load some resources for this executable
	 * only if it's not already present.
	 * We loosely test all the values because no set in glib
	 * (we could use the hash table for that, but meh). - remy */
	gboolean existing = FALSE;
	for (unsigned int i = 0; i < g_queue_get_length(data->cache_executables_id); i++) {
		int* val = g_queue_peek_nth(data->cache_executables_id, i);
		if (*val == data->selected_executable) {
			existing = TRUE;
			break;
		}
	}
	if (!existing) {
		int* idx = g_new(int, 1); *idx = data->selected_executable;
		g_queue_push_tail(data->cache_executables_id, idx);
	}

	return;
} 

/*
 * meh_exec_list_start_executable launches the currently selected executable.
 */
static void meh_exec_list_start_executable(App* app, Screen* screen) {
	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* get the executable selected */
	Executable* executable = g_queue_peek_nth(data->executables, data->selected_executable);

	/* no executables to launch. */
	if (executable == NULL) {
		return;
	}

	/* starts the launch screen */
	Screen* launch_screen = meh_screen_launch_new(app, screen, data->platform, executable, data->logo_widget);
	meh_app_set_current_screen(app, launch_screen, TRUE);

	// TODO(remy): here we could clean resources to free some more ram.

	meh_exec_list_delete_some_cache(screen);

	/* end the transitions for when we're coming back */
	meh_transitions_end(screen->transitions);
}

/*
 * meh_exec_list_after_cursor_move refreshes the screen information
 * after a jump in the executable list.
 */
void meh_exec_list_after_cursor_move(App* app, Screen* screen, int prev_selected_exec) {
	meh_exec_list_select_resources(screen);
	meh_exec_list_load_resources(app, screen);
	meh_exec_list_delete_some_cache(screen);
	meh_exec_list_resolve_tex(screen);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* stops every transitions */
	meh_transitions_end(screen->transitions);
	meh_screen_update_transitions(screen);

	/*
	 * move the selection cursor
	 */

	int selected = data->selected_executable % (MEH_EXEC_LIST_SIZE);
	int prev_selected = prev_selected_exec % (MEH_EXEC_LIST_SIZE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, 130 + prev_selected*32, 130 + (selected*32), 100);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/*
	 * look whether the image is a portrait / landscape image
	 * and change the size of the description / cover in function
	 */

	if (data->logo == -1) {
		/* no logo, use the full height for the description */
		data->description_widget->y.value = 50;
		data->description_widget->h = 280;
	} else {
		/* we have a logo, reduce the descrpition size and animate the logo */
		data->description_widget->y.value = 180;
		data->description_widget->h = 150;
		data->logo_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 60, 200);
		meh_screen_add_image_transitions(screen, data->logo_widget);
	}

	if (data->cover == -1 || data->cover_widget->texture == NULL) {
		/* no cover, use the full width for the description */
		data->description_widget->w = 650;
		data->cover_widget->texture = NULL;
	} else {
		/* detect the landscape/portrait mode */
		int w = 0,h = 0;
		SDL_QueryTexture(data->cover_widget->texture, NULL, NULL, &w, &h);
		if (w >= h) {
			/* landscape */
			data->cover_widget->x.value = 930;
			data->cover_widget->w.value = 300;
			data->cover_widget->h.value = 200;
			data->logo_widget->w.value = 340;
			data->description_widget->w = 340;
		} else {
			/* portrait */
			data->cover_widget->x.value = 1030;
			data->cover_widget->w.value = 200;
			data->cover_widget->h.value = 300;
			data->logo_widget->w.value = 440;
			data->description_widget->w = 440;
		}
	}

	/* 
	 * refreshes the text widgets about game info.
	 */

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (current_executable != NULL) {
		data->genres_widget->text = current_executable->genres;
		meh_widget_text_reload(app->window, data->genres_widget);
		data->players_widget->text = current_executable->players;
		meh_widget_text_reload(app->window, data->players_widget);
		data->publisher_widget->text = current_executable->publisher;
		meh_widget_text_reload(app->window, data->publisher_widget);
		data->developer_widget->text = current_executable->developer;
		meh_widget_text_reload(app->window, data->developer_widget);
		data->rating_widget->text = current_executable->rating;
		meh_widget_text_reload(app->window, data->rating_widget);
		data->release_date_widget->text = current_executable->release_date;
		meh_widget_text_reload(app->window, data->release_date_widget);
		data->description_widget->text = current_executable->description;
		meh_widget_text_reload(app->window, data->description_widget);
	}

	/*
	 * do we need to refresh the executable widgets ?
	 * only on page changes
	 */

	int relative_new = data->selected_executable%MEH_EXEC_LIST_SIZE;
	int relative_old = prev_selected_exec%MEH_EXEC_LIST_SIZE;
	if ((relative_new == 0 && relative_old != 1) || /* Last -> First */
		/* The two cases of First -> last */
	    (relative_new == MEH_EXEC_LIST_SIZE-1 && relative_old == 0) ||
		(data->selected_executable == data->executables_length-1))	{
		meh_exec_list_refresh_executables_widget(app, screen);
	}

	/* re-creates the video widget overlay */
	
	if (data->exec_list_video != NULL) {
		meh_exec_list_video_destroy(data->exec_list_video);
		data->exec_list_video = NULL;
	}
	data->exec_list_video = meh_exec_list_video_new(app->window, screen, current_executable);

	/* if no video, we'll put the metadata instead. */
	if (!meh_exec_list_video_has_video(data->exec_list_video)) {
		data->genres_l_widget->x = data->players_l_widget->x =
		data->publisher_l_widget->x = data->developer_l_widget->x =
		data->rating_l_widget->x = data->release_date_l_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_l_widget->x.value, 500, 200);
		data->genres_widget->x = data->players_widget->x =
		data->publisher_widget->x = data->developer_widget->x =
		data->rating_widget->x = data->release_date_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_widget->x.value, 800, 200);
	} else {
		data->genres_l_widget->x = data->players_l_widget->x =
		data->publisher_l_widget->x = data->developer_l_widget->x =
		data->rating_l_widget->x = data->release_date_l_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_l_widget->x.value, 870, 200);
		data->genres_widget->x = data->players_widget->x =
		data->publisher_widget->x = data->developer_widget->x =
		data->rating_widget->x = data->release_date_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_widget->x.value, 1070, 200);
	}

	/* starts the transition of the metadata */
	meh_screen_add_text_transitions(screen, data->genres_widget);
	meh_screen_add_text_transitions(screen, data->genres_l_widget);
	meh_screen_add_text_transitions(screen, data->players_widget);
	meh_screen_add_text_transitions(screen, data->players_l_widget);
	meh_screen_add_text_transitions(screen, data->publisher_widget);
	meh_screen_add_text_transitions(screen, data->publisher_l_widget);
	meh_screen_add_text_transitions(screen, data->developer_widget);
	meh_screen_add_text_transitions(screen, data->developer_l_widget);
	meh_screen_add_text_transitions(screen, data->rating_widget);
	meh_screen_add_text_transitions(screen, data->rating_l_widget);
	meh_screen_add_text_transitions(screen, data->release_date_widget);
	meh_screen_add_text_transitions(screen, data->release_date_l_widget);

	/*
	 * anim the bg
	 */

	meh_exec_list_start_bg_anim(screen);

	/*
	 * reset the move of the texts
	 */

	meh_widget_text_reset_move(data->description_widget);

	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_reset_move(g_queue_peek_nth(data->executable_widgets, i));
	}
}

/*
 * meh_exec_list_refresh_executables_widget re-creates all the texture
 * in the text widgets for the executables.
 */
void meh_exec_list_refresh_executables_widget(App* app, Screen* screen) {
	ExecutableListData* data = meh_exec_list_get_data(screen);

	int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE));

	/* for every executable text widget */
	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		WidgetText* text = g_queue_peek_nth(data->executable_widgets, i);
		text->text = "";
		
		/* look for the executable text if any */
		int executable_idx = page*(MEH_EXEC_LIST_SIZE) + i;
		if (executable_idx <= data->executables_length) {
			Executable* executable = g_queue_peek_nth(data->executables, executable_idx);
			if (executable != NULL) {
				text->text = executable->display_name;
			}
		}

		/* reload the text texture. */
		meh_widget_text_reload(app->window, text);
	}
}

static void meh_exec_list_open_popup(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);
	Executable* executable = g_queue_peek_nth(data->executables, data->selected_executable);

	if (executable == NULL) {
		return;
	}

	/* create the child screen */
	Screen* popup_screen = meh_screen_popup_new(app, screen, executable);
	meh_app_set_current_screen(app, popup_screen, TRUE);
	/* NOTE we don't free the memory of the current screen, the popup screen
	 * will go back to it later. */
}

/*
 * meh_exec_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_exec_list_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	int prev_selected_exec = data->selected_executable;

	switch (pressed_button) {
		case MEH_INPUT_SPECIAL_ESCAPE:
			/* exit mehstation */
			app->mainloop.running = FALSE;
			break;
		case MEH_INPUT_BUTTON_B:
			if (screen->parent_screen != NULL) {
				/* back to the platform screen using a fade
				 * screen for the transition */
				Screen* fade_screen = meh_screen_fade_new(app, screen, screen->parent_screen);
				meh_app_set_current_screen(app, fade_screen, TRUE);
				/* NOTE we don't free the memory of the exec list screen, the fade screen
				 * will do it. */
			}
			break;
		case MEH_INPUT_BUTTON_START:
			/* start the popup */
			meh_exec_list_open_popup(app, screen);
			break;
		case MEH_INPUT_BUTTON_A:
			/* launch the game */
			meh_exec_list_start_executable(app, screen);
			break;
		case MEH_INPUT_BUTTON_UP:
			if (data->selected_executable == 0) {
				data->selected_executable = data->executables_length-1;
			} else {
				data->selected_executable -= 1;
			}
			meh_exec_list_after_cursor_move(app, screen, prev_selected_exec);
			break;
		case MEH_INPUT_BUTTON_DOWN:
			if (data->selected_executable == data->executables_length-1) {
				data->selected_executable = 0;
			} else {
				data->selected_executable += 1;
			}
			meh_exec_list_after_cursor_move(app, screen, prev_selected_exec);
			break;
		case MEH_INPUT_BUTTON_L:
			{
				int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE)) - 1;
				data->selected_executable = page * MEH_EXEC_LIST_SIZE;
				if (data->selected_executable < 0) {
					data->selected_executable = data->executables_length-1;
				}
				meh_exec_list_after_cursor_move(app, screen, prev_selected_exec);
			}
			break;
		case MEH_INPUT_BUTTON_R:
			{
				int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE)) + 1;
				data->selected_executable = page * MEH_EXEC_LIST_SIZE;
				if (data->selected_executable > data->executables_length) {
					data->selected_executable = 0;
				}
				meh_exec_list_after_cursor_move(app, screen, prev_selected_exec);
			}
			break;
	}
}

/*
 * meh_exec_list_update updates the executable list.
 */
int meh_exec_list_update(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* updates all the transition in the screen */
	meh_screen_update_transitions(screen);

	/* updates the text of the selected game */

	int selected = data->selected_executable % (MEH_EXEC_LIST_SIZE);
	WidgetText* t = g_queue_peek_nth(data->executable_widgets, selected);
	if (t != NULL) {
		meh_widget_text_update(screen, t);
	}

	meh_widget_text_update(screen, data->description_widget);

	meh_widget_text_update(screen, data->genres_widget);
	meh_widget_text_update(screen, data->rating_widget);
	meh_widget_text_update(screen, data->publisher_widget);
	meh_widget_text_update(screen, data->release_date_widget);
	meh_widget_text_update(screen, data->developer_widget);
	meh_widget_text_update(screen, data->players_widget);

	meh_exec_list_video_update(screen, data->exec_list_video);

	return 0;
}

/*
 * meh_exec_list_resolve_tex resolves all the tex index
 * to real textures for object referencing them.
 */
static void meh_exec_list_resolve_tex(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	if (data->background > -1) {
		data->background_widget->texture = g_hash_table_lookup(data->textures, &(data->background));
	}
	if (data->cover > -1) {
		data->cover_widget->texture = g_hash_table_lookup(data->textures, &(data->cover));
	}
	if (data->logo > -1) {
		data->logo_widget->texture = g_hash_table_lookup(data->textures, &(data->logo));
	}
	for (int i = 0; i < 3; i++) {
		if (data->screenshots[i] > -1) {
			data->screenshots_widget[i]->texture = g_hash_table_lookup(data->textures, &(data->screenshots[i]));
		}
	}
}

/*
 * meh_exec_list_render renders the executable list view.
 */
int meh_exec_list_render(App* app, Screen* screen, gboolean flip) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);
	
	ExecutableListData* data = meh_exec_list_get_data(screen);

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);

	/* background */
	meh_widget_image_render(app->window, data->background_widget);

	/* background hover */
	meh_widget_rect_render(app->window, data->bg_hover_widget);

	/* header */
	meh_widget_text_render(app->window, data->header_text_widget);

	/* selection */
	meh_widget_rect_render(app->window, data->selection_widget);

	/* cover */
	if (data->cover != -1) {
		meh_widget_image_render(app->window, data->cover_widget);
	}

	/* logo */
	if (data->logo != -1) {
		meh_widget_image_render(app->window, data->logo_widget);
	}

	/* render the screenshots */
	for (int i = 0; i < 3; i++) {
		meh_widget_image_render(app->window, data->screenshots_widget[i]);
	}

	/*
	 * extra info
	 */
	if (current_executable != NULL) {
		/* Genres */
		meh_widget_text_render(app->window, data->genres_l_widget);
		meh_widget_text_render(app->window, data->genres_widget);
		/* Players */
		meh_widget_text_render(app->window, data->players_l_widget);
		meh_widget_text_render(app->window, data->players_widget);
		/* Publisher */
		meh_widget_text_render(app->window, data->publisher_l_widget);
		meh_widget_text_render(app->window, data->publisher_widget);
		/* Developer */
		meh_widget_text_render(app->window, data->developer_l_widget);
		meh_widget_text_render(app->window, data->developer_widget);
		/* Release date */
		meh_widget_text_render(app->window, data->rating_l_widget);
		meh_widget_text_render(app->window, data->rating_widget);
		/* Release date */
		meh_widget_text_render(app->window, data->release_date_l_widget);
		meh_widget_text_render(app->window, data->release_date_widget);
		/* Description */
		meh_widget_text_render(app->window, data->description_widget);
	}

	/* render all the executables names. */
	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_render(app->window, g_queue_peek_nth(data->executable_widgets, i));
	}

	/* video overlay */

	meh_exec_list_video_render(app->window, data->exec_list_video);

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
	return 0;
}
