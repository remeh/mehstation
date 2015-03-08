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

#define MEH_EXEC_LIST_MAX_CACHE (7)
#define MEH_EXEC_LIST_DELTA (3) /* Don't delete the cache of the object around the cursor */

#define MEH_EXEC_LIST_SIZE (17) /* Maximum amount of executables displayed */

static void meh_screen_exec_create_widgets(App* app, Screen* screen, ExecutableListData* data);
static void meh_screen_exec_list_destroy_resources(Screen* screen);
static void meh_screen_exec_list_load_resources(App* app, Screen* screen);
static void meh_screen_exec_list_start_executable(App* app, Screen* screen);
static void meh_screen_exec_list_select_resources(Screen* screen);
static void meh_screen_exec_list_start_bg_anim(Screen* screen);
static void meh_screen_exec_list_refresh_executables_widget(App* app, Screen* screen);

Screen* meh_screen_exec_list_new(App* app, int platform_id) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new();

	screen->name = g_strdup("Executable list screen");
	screen->messages_handler = &meh_screen_exec_list_messages_handler;
	screen->destroy_data = &meh_screen_exec_list_destroy_data;

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
	data->header_text = g_utf8_strup(data->platform->name, -1);

	data->executable_widgets = g_queue_new();

	/* create widgets */
	meh_screen_exec_create_widgets(app, screen, data);

	screen->data = data;

	/*
	 * Select and load the first resources
	 */
	meh_screen_exec_list_select_resources(screen);
	meh_screen_exec_list_load_resources(app, screen);
	meh_screen_exec_list_start_bg_anim(screen);
	meh_screen_exec_list_refresh_executables_widget(app, screen);

	return screen;
}

static void meh_screen_exec_create_widgets(App* app, Screen* screen, ExecutableListData* data) {
	g_assert(app != NULL);
	g_assert(screen != NULL);
	g_assert(data != NULL);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color white_transparent = { 255, 255, 255, 50 };
	SDL_Color gray = { 10, 10, 10, 245 };

	/* Selection. */
	data->selection_widget = meh_widget_rect_new(40, -100, 550, 28, white_transparent, TRUE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 130, 500);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/* List background */
	data->list_background_widget = meh_widget_rect_new(35, 125, 1210, 550, gray, TRUE);

	/* Background */
	data->background_widget = meh_widget_image_new(NULL, -50, -50, app->window->width+50, app->window->height+50);

	/* Background hover */
	data->background_hover_widget = meh_widget_rect_new(0, 0, app->window->width, app->window->height, white_transparent, TRUE); 

	/* Header */
	data->header_text_widget = meh_widget_text_new(app->big_font, data->header_text, 20, 10, white, TRUE);
	data->header_widget = meh_widget_rect_new(0, 0, app->window->width, 70, gray, TRUE);

	/* Executables */
	for (int i = 0; i < MEH_EXEC_LIST_SIZE; i++) {
		WidgetText* text = meh_widget_text_new(app->small_font, "", 60, 130+(i*32), white, FALSE);
		g_queue_push_tail(data->executable_widgets, text);
	}

	/*
	 * Extra information
	 */

	/* Genres */
	data->genres_l_widget = meh_widget_text_new(app->small_bold_font, "Genres", 580, 558, white, TRUE);
	data->genres_widget = meh_widget_text_new(app->small_font, NULL, 710, 560, white, TRUE);

	/* Players */
	data->players_l_widget = meh_widget_text_new(app->small_bold_font, "Players", 870, 558, white, TRUE);
	data->players_widget = meh_widget_text_new(app->small_font, NULL, 1030, 560, white, TRUE);

	/* Publisher */
	data->publisher_l_widget = meh_widget_text_new(app->small_bold_font, "Publisher", 580, 588, white, TRUE);
	data->publisher_widget = meh_widget_text_new(app->small_font, NULL, 710, 590, white, TRUE);

	/* Developer */
	data->developer_l_widget = meh_widget_text_new(app->small_bold_font, "Developer", 870, 588, white, TRUE);
	data->developer_widget = meh_widget_text_new(app->small_font, NULL, 1030, 590, white, TRUE);

	/* Rating */
	data->rating_l_widget = meh_widget_text_new(app->small_bold_font, "Rating", 580, 618, white, TRUE);
	data->rating_widget = meh_widget_text_new(app->small_font, NULL, 710, 620, white, TRUE);

	/* Release date */
	data->release_date_l_widget = meh_widget_text_new(app->small_bold_font, "Release date", 870, 618, white, TRUE);
	data->release_date_widget = meh_widget_text_new(app->small_font, NULL, 1030, 620, white, TRUE);

	/* Cover */
	data->cover_widget = meh_widget_image_new(NULL, 1030, 140, 200, 300);
}

/*
 * meh_screen_exec_list_start_bg_anim slighty moves the background to give
 * an impression of dynamic.
 */
static void meh_screen_exec_list_start_bg_anim(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	g_assert(data != NULL);

	data->background_widget->x = meh_transition_start(MEH_TRANSITION_LINEAR, -50, 0, 10000);
	data->background_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, -50, 0, 10000);
	meh_screen_add_image_transitions(screen, data->background_widget);
}

/*
 * meh_screen_exec_list_destroy_data role is to delete the typed data of the screen
 */
void meh_screen_exec_list_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	if (data != NULL) {
		meh_model_platform_destroy(data->platform);
		meh_model_executables_destroy(data->executables);

		/* Destroy the widgets */
		meh_widget_image_destroy(data->background_widget);
		meh_widget_rect_destroy(data->header_widget);
		meh_widget_rect_destroy(data->selection_widget);
		meh_widget_rect_destroy(data->background_hover_widget);
		meh_widget_rect_destroy(data->list_background_widget);
		meh_widget_image_destroy(data->cover_widget);
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

		for (int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
			meh_widget_text_destroy( g_queue_peek_nth( data->executable_widgets, i) );
		}
		g_queue_free(data->executable_widgets);

		/* Free the executables id cache. */
		int i = 0;
		for (i = 0; i < g_queue_get_length(data->cache_executables_id); i++) {
			g_free(g_queue_peek_nth(data->cache_executables_id, i));
		}
		g_queue_free(data->cache_executables_id);

		/* Misc */
		g_free(data->header_text);

		/* We must free the textures cache */
		meh_screen_exec_list_destroy_resources(screen);
	}
}

/*
 * meh_screen_exec_list_destroy_resources takes care of destroying the
 * resources.
 */
static void meh_screen_exec_list_destroy_resources(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);

	if (data == NULL || data->textures == NULL) {
		return;
	}

	int i;
	GList* keys = g_hash_table_get_keys(data->textures);
	for (i = 0; i < g_list_length(keys); i++) {
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
 * meh_screen_exec_list_is_in_delta checks whether the given index is in the delta
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
static gboolean meh_screen_exec_list_is_in_delta(ExecutableListData* data, int idx) {
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
 * meh_screen_exec_list_delete_some_cache looks for which cache we could
 * free without impacting the user experience.
 */
static void meh_screen_exec_list_delete_some_cache(Screen* screen) {
	g_assert(screen != NULL);	

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	if (data == NULL) {
		return;
	}

	/* Get the current executable */
	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (current_executable == NULL || current_executable->resources == NULL) {
		return;
	}

	int watchdog = 20; /* Avoid an infinite loop. */
	while (g_queue_get_length(data->cache_executables_id) > MEH_EXEC_LIST_MAX_CACHE && watchdog > 0) {
		int* idx = g_queue_pop_head(data->cache_executables_id);
		if (*idx != current_executable->id &&
			!meh_screen_exec_list_is_in_delta(data, *idx)) { /* do not free the resources of the current selection */
			/* executable for which we want to free the resources */
			Executable* exec_to_clear_for = g_queue_peek_nth(data->executables, *idx);
			if (exec_to_clear_for != NULL && exec_to_clear_for->resources != NULL) {
				g_debug("Cache cleaning of the resources of %s", exec_to_clear_for->display_name);
				/* free the resources of this executable */
				int i = 0;
				for (i = 0; i < g_queue_get_length(exec_to_clear_for->resources); i++) {
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

	g_debug("%d", g_queue_get_length(data->cache_executables_id));
}

/*
 * meh_screen_exec_list_select_resources uses the resources of the currently selected
 * executable to select a background and a cover.
 */
static void meh_screen_exec_list_select_resources(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	if (data == NULL) {
		return;
	}

	Executable* executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (executable == NULL || executable->resources == NULL) {
		return;
	}

	/*
	 * Select a random resource if any and tries to not take a cover if possible.
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

		if (g_strcmp0(resource->type, "cover") != 0) {
			/* We found something that's not a cover, perfect. */
			break;
		} else {
			if (g_queue_get_length(executable->resources) == 1) {
				/* We only found a cover, but we have only one resource, so we can also stop here. */
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
	 * Select a cover
	 */
	int i = 0;
	for (i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* res = g_queue_peek_nth(executable->resources, i++);
		if (res != NULL) {
			if (g_strcmp0(res->type, "cover") == 0) {
				data->cover = res->id;
				g_debug("Selected cover: %d", res->id);
				break;
			}
		}
	}
}

/*
 * meh_screen_exec_list_get_data returns the data of the executable_list screen
 */
ExecutableListData* meh_screen_exec_list_get_data(Screen* screen) {
	ExecutableListData* data = (ExecutableListData*) screen->data;
	g_assert(data != NULL);
	return data;
}

int meh_screen_exec_list_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}


	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				int* pressed_button = (int*)message->data;
				meh_screen_exec_list_button_pressed(app, screen, *pressed_button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_exec_list_update(screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				if (message->data == NULL) {
					meh_screen_exec_list_render(app, screen, TRUE);
				} else {
					gboolean* flip = (gboolean*)message->data;
					meh_screen_exec_list_render(app, screen, *flip);
				}
			}
			break;
	}

	return 0;
}

/*
 * meh_screen_exec_list_load_resources loads the resources of the currently
 * selected game.
 */
static void meh_screen_exec_list_load_resources(App* app, Screen* screen) {
	g_assert(app != NULL);	
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);

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
	int i = 0;
	for (i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* resource = g_queue_peek_nth(executable->resources, i);
		if (resource == NULL) {
			continue;
		}

		/* Load only the needed resources. */
		if (resource->id != data->background && resource->id != data->cover) {
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
	for (i = 0; i < g_queue_get_length(data->cache_executables_id); i++) {
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
 * meh_screen_exec_list_start_executable launches the currently selected executable.
 */
static void meh_screen_exec_list_start_executable(App* app, Screen* screen) {
	ExecutableListData* data = meh_screen_exec_list_get_data(screen);

	/* get the executable selected */
	Executable* executable = g_queue_peek_nth(data->executables, data->selected_executable);

	/* no executables to launch. */
	if (executable == NULL) {
		return;
	}

	/* prepare the exec call */
	gchar** command_parts = g_strsplit(data->platform->command, " ", -1);
	/* replace the flags */
	int i = 0;
	while (1) {
		if (command_parts[i] == NULL) {
			break;
		}
		/* FIXME Sure that there's not utf8 problems there ? */
		if (g_strcmp0(command_parts[i], "\%exec\%") == 0) {
			/* remove the placeholder */
			g_free(command_parts[i]);
			/* replace by the value */
			command_parts[i] = g_strdup( executable->filepath );
		}
		i++;
	}

	const gchar* working_dir = "/usr/bin"; /* FIXME what about executable not in /usr/bin */
	int exit_status = 0;
	GError* error = NULL;
	g_spawn_sync(working_dir,
				 command_parts,
				 NULL,
				 G_SPAWN_DEFAULT,
				 NULL,
				 NULL,
				 NULL,
				 NULL,
				 &exit_status,
				 &error);

	if (error != NULL) {
		g_critical("can't start the platform '%s' with the executable '%s' with command '%s'.", data->platform->name, executable->display_name, data->platform->command);
		g_critical("%s", error->message);
		g_error_free(error);
	}

	/* release used memory */
	g_strfreev(command_parts);

	/* when launching something, we may have missed some
	 * input events, reset everything in case of. */
	meh_input_manager_reset_buttons_state(app->input_manager);
	/* FIXME When returning from the other app, if the user presses the same
	 * FIXME key as the one used to start the system, SDL will considered it
	 * FIXME as a repeatition. We should maybe generate a fake KEYUP event ?*/
	SDL_RaiseWindow(app->window->sdl_window);
}

/*
 * meh_screen_exec_list_refresh_after_cursor_move refreshes the screen information
 * after a jump in the executable list.
 */
static void meh_screen_exec_list_refresh_after_cursor_move(App* app, Screen* screen, int prev_selected_exec) {
	meh_screen_exec_list_select_resources(screen);
	meh_screen_exec_list_load_resources(app, screen);
	meh_screen_exec_list_delete_some_cache(screen);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);

	int selected = data->selected_executable % (MEH_EXEC_LIST_SIZE);
	int prev_selected = prev_selected_exec % (MEH_EXEC_LIST_SIZE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, 130 + prev_selected*32, 130 + (selected*32), 100);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/* Refreshes the text widgets about game info. */
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
	}

	/* do we need to refresh the executable widgets ?
	 * Only on page changes */
	int relative_new = data->selected_executable%MEH_EXEC_LIST_SIZE;
	int relative_old = prev_selected_exec%MEH_EXEC_LIST_SIZE;
	if ((relative_new == 0 && relative_old != 1) || /* Last -> First */
		/* TODO Doesn't work with incomplete list. */
	    (relative_new == MEH_EXEC_LIST_SIZE-1 && relative_old == 0))	{
		meh_screen_exec_list_refresh_executables_widget(app, screen);
	}

	meh_screen_exec_list_start_bg_anim(screen);
}

/*
 * meh_screen_exec_list_refresh_executables_widget re-creates all the texture
 * in the text widgets for the executables.
 */
static void meh_screen_exec_list_refresh_executables_widget(App* app, Screen* screen) {
	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	g_assert(data != NULL);

	int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE));

	/* For every executable text widget */
	for (int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
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

		// reload the text texture.
		meh_widget_text_reload(app->window, text);
	}
}

/*
 * meh_screen_exec_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_exec_list_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	g_assert(data != NULL);

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
				meh_app_set_current_screen(app, fade_screen);
				/* NOTE we don't free the memory of the starting screen, the fade screen
				 * will do it. */
			}
			break;
		case MEH_INPUT_BUTTON_A:
			/* launch the game */
			meh_screen_exec_list_start_executable(app, screen);
			break;
		case MEH_INPUT_BUTTON_UP:
			if (data->selected_executable == 0) {
				data->selected_executable = data->executables_length-1;
			} else {
				data->selected_executable -= 1;
			}
			meh_screen_exec_list_refresh_after_cursor_move(app, screen, prev_selected_exec);
			break;
		case MEH_INPUT_BUTTON_DOWN:
			if (data->selected_executable == data->executables_length-1) {
				data->selected_executable = 0;
			} else {
				data->selected_executable += 1;
			}
			meh_screen_exec_list_refresh_after_cursor_move(app, screen, prev_selected_exec);
			break;
		case MEH_INPUT_BUTTON_L:
			{
				int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE)) - 1;
				data->selected_executable = page * MEH_EXEC_LIST_SIZE;
				if (data->selected_executable < 0) {
					data->selected_executable = data->executables_length-1;
				}
				meh_screen_exec_list_refresh_after_cursor_move(app, screen, prev_selected_exec);
			}
			break;
		case MEH_INPUT_BUTTON_R:
			{
				int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE)) + 1;
				data->selected_executable = page * MEH_EXEC_LIST_SIZE;
				if (data->selected_executable > data->executables_length) {
					data->selected_executable = 0;
				}
				meh_screen_exec_list_refresh_after_cursor_move(app, screen, prev_selected_exec);
			}
			break;
	}
}

/*
 * meh_screen_exec_list_update updates the executable list.
 */
int meh_screen_exec_list_update(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	g_assert(data != NULL);

	/* updates all the transition in the screen */
	meh_screen_update_transitions(screen);

	return 0;
}

/*
 * meh_screen_exec_list_render renders the executable list view.
 */
int meh_screen_exec_list_render(App* app, Screen* screen, gboolean flip) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);
	
	ExecutableListData* data = meh_screen_exec_list_get_data(screen);
	g_assert(data != NULL);

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);

	/* background */
	if (data->background > -1) {
		data->background_widget->texture = g_hash_table_lookup(data->textures, &(data->background));
	}
	meh_widget_image_render(app->window, data->background_widget);

	/* background hover */
	meh_widget_rect_render(app->window, data->background_hover_widget);

	/* header */
	meh_widget_rect_render(app->window, data->header_widget);
	meh_widget_text_render(app->window, data->header_text_widget);

	/* list background */
	meh_widget_rect_render(app->window, data->list_background_widget);

	/* selection */
	meh_widget_rect_render(app->window, data->selection_widget);

	/* cover */
	if (data->cover > -1) {
		data->cover_widget->texture = g_hash_table_lookup(data->textures, &(data->cover));
	}
	meh_widget_image_render(app->window, data->cover_widget);

	/*
	 * Extra info
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
	}

	/* Render all the executables names. */
	for (int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_render(app->window, g_queue_peek_nth(data->executable_widgets, i));
	}

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
	return 0;
}

