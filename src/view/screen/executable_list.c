/*
 * mehstation - Screen of executables.
 *
 * Copyright © 2015 Rémy Mathieu
 *
 * TODO(remy): this part is the more complicated of mehstation,
 * it should probably be re-done with better code (for example the
 * support of many modes appeared lately in the code).
 */

#include <glib.h>

#include "system/app.h"
#include "system/audio.h"
#include "system/consts.h"
#include "system/db.h"
#include "system/discover.h"
#include "system/input.h"
#include "system/message.h"
#include "system/transition.h"
#include "system/db/models.h"
#include "view/image.h"
#include "view/widget_text.h"
#include "view/screen.h"
#include "view/screen/fade.h"
#include "view/screen/executable_list.h"
#include "view/screen/platform_list.h"
#include "view/screen/exec_desc.h"
#include "view/screen/exec_complete_selec.h"
#include "view/screen/exec_cover_selec.h"
#include "view/screen/simple_popup.h"
#include "view/screen/exec_popup.h"

static void meh_exec_create_widgets(App* app, Screen* screen, ExecutableListData* data);
static void meh_exec_list_destroy_resources(Screen* screen);
static void meh_exec_list_load_resources(App* app, Screen* screen);
static void meh_exec_list_start_executable(App* app, Screen* screen);
static void meh_exec_list_select_resources(Screen* screen);
static void meh_exec_list_resolve_tex(Screen* screen);
static void meh_exec_list_init_widgets_to_null(ExecutableListData* data);

#define MEH_PLATFORM_BG_ID ((2^31)-2)

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

	/* maybe we need to automatically discover some executables ? */
	if (data->platform->discover_dir != NULL &&
			g_utf8_strlen(data->platform->discover_dir, -1) > 0) {
		g_debug("Starting directory scan for platform %s (in directory '%s' with ext '%s')",
				data->platform->name,
				data->platform->discover_dir,
				data->platform->discover_ext);
		meh_discover_scan_directory(app, data->platform);
	}

	/* get the executables */
	data->executables = meh_db_get_platform_executables(app->db, data->platform, TRUE);
	data->executables_length = g_queue_get_length(data->executables);
	data->cache_executables_id = g_queue_new();
	data->selected_executable = 0;

	/* init every widgets to NULL */
	meh_exec_list_init_widgets_to_null(data);

	/* display resources */

	data->background = -1;
	data->cover = -1;
	data->logo = -1;
	data->screenshots[0] = data->screenshots[1] = data->screenshots[2] = -1;

	data->executable_widgets = g_queue_new();

	data->exec_list_video = NULL;
	data->show_descriptions = TRUE;

	screen->data = data;

	data->textures = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, NULL);

	/* adds the platform texture if any with the special ID MEH_PLATFORM_BG_ID */
	if (g_utf8_strlen(data->platform->background, 2) > 0) {
		SDL_Texture* platform_tex = meh_image_load_file(app->window->sdl_renderer, data->platform->background);
		if (platform_tex != NULL) {
			int *id = g_new(int, 1); *id = MEH_PLATFORM_BG_ID;
			g_hash_table_insert(data->textures, id, platform_tex);
		}
	}

	/* create widgets */
	meh_exec_create_widgets(app, screen, data);

	/*
	 * Select and load the first resources
	 * then refresh all the text textures by faking
	 * a cursor movement.
	 */
	meh_exec_list_after_cursor_move(app, screen, -1);

	return screen;
}

void meh_exec_list_init_widgets_to_null(ExecutableListData* data) {
	data->selection_widget =
	data->bg_hover_widget = NULL;
	data->cover_widget =
	data->logo_widget =
	data->favorite_widget =
	data->background_widget =
	data->screenshots_widget[0] =
	data->screenshots_widget[1] =
	data->screenshots_widget[2] = NULL;
	data->header_text_widget =
	data->genres_widget =
	data->genres_l_widget =
	data->players_widget =
	data->players_l_widget =
	data->publisher_widget =
	data->publisher_l_widget =
	data->developer_widget =
	data->developer_l_widget =
	data->rating_widget =
	data->rating_l_widget =
	data->release_date_widget =
	data->release_date_l_widget =
	data->description_widget =
	data->prev_executable_widget =
	data->next_executable_widget =
	data->executable_title_widget = NULL;
	data->exec_list_video = NULL;
}

static void meh_exec_create_widgets(App* app, Screen* screen, ExecutableListData* data) {
	g_assert(app != NULL);
	g_assert(screen != NULL);
	g_assert(data != NULL);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color gray = { 10, 10, 10, 200 };

	/* Background */
	data->background_widget = meh_widget_image_new(NULL, -50, -50, MEH_FAKE_WIDTH+50, MEH_FAKE_HEIGHT+50);

	/* Background hover */
	data->bg_hover_widget = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, gray, TRUE); 

	/* Favorite */
	gchar* tmp = g_strdup_printf("%s/favorite.png", app->res_dir);
	SDL_Texture* favorite = meh_image_load_file(app->window->sdl_renderer, tmp);
	g_free(tmp);
	tmp = NULL;
	data->favorite_widget = meh_widget_image_new(favorite, -200, -200, 50, 50); /* hidden favorite image */

	/* Header */
	data->header_text_widget = meh_widget_text_new(app->big_font, data->platform->name, 20, 55, 400, 40, white, TRUE);
	data->header_text_widget->uppercase = TRUE;
	data->header_text_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, -200, 20, 300);
	meh_screen_add_text_transitions(screen, data->header_text_widget);

	/* Complete / cover mode */
	if (g_strcmp0(data->platform->type, "complete") == 0) {
		meh_complete_selec_create_widgets(app, screen);
		meh_exec_desc_create_widgets(app, screen);
	} else {
		meh_cover_selec_create_widgets(app, screen);
	}
}

/*
 * meh_exec_list_destroy_data role is to delete the typed data of the screen
 */
void meh_exec_list_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);
	if (data != NULL) {

		/* background and header */
		if (data->background_widget != NULL) {
			meh_widget_image_destroy(data->background_widget);
			data->background_widget = NULL;
		}
		if (data->bg_hover_widget != NULL) {
			meh_widget_rect_destroy(data->bg_hover_widget);
			data->bg_hover_widget = NULL;
		}
		if (data->header_text_widget != NULL) {
			meh_widget_text_destroy(data->header_text_widget);
			data->header_text_widget = NULL;
		}

		/* favorite */
		if (data->favorite_widget != NULL) {
			SDL_DestroyTexture(data->favorite_widget->texture);
			data->favorite_widget->texture = NULL;
			meh_widget_image_destroy(data->favorite_widget);
			data->favorite_widget = NULL;
		}

		/* free the executables id cache. */
		for (unsigned int i = 0; i < g_queue_get_length(data->cache_executables_id); i++) {
			g_free(g_queue_peek_nth(data->cache_executables_id, i));
		}
		g_queue_free(data->cache_executables_id);

		if (g_strcmp0(data->platform->type, "complete") == 0) {
			/* destroy the selection widgets */
			meh_complete_selec_destroy(screen);

			/* destroy executable description */
			meh_exec_desc_destroy(screen);
		} else {
			meh_cover_selec_destroy(screen);
		}

		/* destroy the video overlay */
		meh_exec_list_video_destroy(data->exec_list_video);
		data->exec_list_video = NULL;

		/* finally destroy the models */
		meh_model_platform_destroy(data->platform);
		meh_model_executables_destroy(data->executables);

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
	data->textures = NULL;
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
 * executable to select a background, a cover, a logo, fanarts, screenshots
 * and a video, if available.
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
	data->background = -1;
	data->screenshots[0] = data->screenshots[1] = data->screenshots[2] = -1;
	if (data->cover_widget != NULL) {
		data->cover_widget->texture = NULL;
	}

	if (data->logo_widget != NULL) {
		data->logo_widget->texture = NULL;
	}

	if (data->screenshots_widget[0] != NULL) {
		data->screenshots_widget[0]->texture = NULL;
	}
	if (data->screenshots_widget[1] != NULL) {
		data->screenshots_widget[1]->texture = NULL;
	}
	if (data->screenshots_widget[2] != NULL) {
		data->screenshots_widget[2]->texture = NULL;
	}

	/*
	 * Select a random resource if any and tries to not take a cover nor a logo if possible.
	 */
	int length = g_queue_get_length(executable->resources);
	if (length == 0) {
		/* no resources are available for this executable,
		 * try to fallback on the platform background. */
		if (g_utf8_strlen(data->platform->background, 2) > 0) {
			data->background = MEH_PLATFORM_BG_ID; /* use the special platform background ID: -1 */
			g_debug("Selected background : platform background.");
		}
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
	 * Select a cover and logo.
	 * Also, creates a list of indexes of screenshots/fanarts.
	 */

	GQueue* shots_and_fanarts = g_queue_new();

	for (unsigned int i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* res = g_queue_peek_nth(executable->resources, i);
		if (res != NULL) {
			if (g_strcmp0(res->type, "cover") == 0) {
				data->cover = res->id;
				g_debug("Selected cover: %d", res->id);
			} else if (g_strcmp0(res->type, "logo") == 0) {
				data->logo = res->id;
				g_debug("Selected logo: %d", res->id);
			} else if (g_strcmp0(res->type, "screenshot") == 0 ||
					   g_strcmp0(res->type, "fanart") == 0) {
				int *value = g_new(int, 1);
				*value = res->id;
				g_queue_push_tail(shots_and_fanarts, value);
			}
		}
	}

	/* selects some screenshots or fanarts */
	for (unsigned int i = 0; i < 3 && g_queue_get_length(shots_and_fanarts) > 0; i++) { /* 3 max or no more values in the queue */
		int idx = g_random_int_range(0, g_queue_get_length(shots_and_fanarts));

		int *value = g_queue_pop_nth(shots_and_fanarts, idx);
		data->screenshots[i] = *value;
		g_free(value);
	}

	/* empty the left values */
	for (unsigned int i = 0; i < g_queue_get_length(shots_and_fanarts); i++) {
		g_free(g_queue_peek_nth(shots_and_fanarts, i));
	}
	g_queue_free(shots_and_fanarts);
}

/*
 * meh_exec_list_get_data returns the data of the executable_list screen
 */
ExecutableListData* meh_exec_list_get_data(Screen* screen) {
	g_assert(screen != NULL);
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
	Executable* exec = g_queue_peek_nth(data->executables, data->selected_executable);

	/* no executables to launch. */
	if (exec == NULL) {
		return;
	}

	/* keep some infos */
	PlatformListData* platform_list_data = meh_screen_platform_list_get_data(screen->parent_screen);

	int cursor_platform = platform_list_data->selected_platform;
	int cursor_exec = data->selected_executable;

	/* Copy the models */
	Platform* platform = meh_model_platform_copy(data->platform);
	Executable* executable = meh_model_executable_copy(exec);

	/* destroy screens view */
	meh_screen_destroy(screen->parent_screen);
	screen->parent_screen = NULL;
	platform_list_data = NULL;
	meh_screen_destroy(screen);
	app->current_screen = NULL;
	data = NULL;

	/* starts the executable */

	meh_app_start_executable(app, platform, executable);

	/* recreate the executable list view */
	screen = meh_exec_list_new(app, platform->id);
	data = meh_exec_list_get_data(screen);

	/* restore the cursor position */
	data->selected_executable = cursor_exec;
	meh_exec_list_after_cursor_move(app, screen, 0);

	meh_app_set_current_screen(app, screen, TRUE);

	/* recreate the parent screen which is the platform list */
	Screen* platform_screen = meh_screen_platform_list_new(app);
	screen->parent_screen = platform_screen;

	/* restore the cursor position */
	platform_list_data = meh_screen_platform_list_get_data(platform_screen);
	platform_list_data->selected_platform = cursor_platform;
	meh_screen_platform_change_platform(app, platform_screen);

	/* free the copied models */
	meh_model_executable_destroy(executable);
	meh_model_platform_destroy(platform);
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

	/* adapt the executable description view. */
	if (g_strcmp0(data->platform->type, "complete") == 0) {
		meh_exec_desc_adapt_view(app, screen);

		/*
		 * do we need to refresh the executable widgets ?
		 * only on page changes
		 * TODO(remy): put this in the exec_selection part ?
		 */

		int page_new = data->selected_executable/MEH_EXEC_LIST_SIZE;
		int page_old = prev_selected_exec/MEH_EXEC_LIST_SIZE;
		if (prev_selected_exec == -1 || page_new != page_old) {
			meh_complete_selec_refresh_executables_widgets(app, screen);
		}
	} else {
		meh_cover_selec_adapt_view(app, screen, prev_selected);
	}


	/*
	 * animate a fade on the background if it's not the platform background.
	 */

	if (data->background > -1 && data->background != MEH_PLATFORM_BG_ID) {
		data->bg_hover_widget->a = meh_transition_start(MEH_TRANSITION_CUBIC, 255, 200, 300);
		meh_screen_add_rect_transitions(screen, data->bg_hover_widget);
	}

	/*
	 * reset the move of the texts
	 */

	if (data->description_widget != NULL) {
		meh_widget_text_reset_move(data->description_widget);
	}

	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_reset_move(g_queue_peek_nth(data->executable_widgets, i));
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
	Screen* popup_screen = meh_simple_popup_new(app, screen, data->platform, executable);
	meh_simple_popup_add_action(
			app,
			popup_screen,
			g_strdup("Toggle favorite"),
			&meh_exec_popup_favorite_toggle
			);
	meh_simple_popup_add_action(
			app,
			popup_screen,
			g_strdup("Run random executable"),
			&meh_exec_popup_start_random_executable
			);
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
		case MEH_INPUT_BUTTON_B:
			if (screen->parent_screen != NULL) {
				meh_audio_play(app->audio, SFX_BACK);
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
			meh_audio_play(app->audio, SFX_BACK);
			meh_exec_list_open_popup(app, screen);
			break;
		case MEH_INPUT_BUTTON_A:
			/* launch the game */
			meh_exec_list_start_executable(app, screen);
			break;
		case MEH_INPUT_BUTTON_LEFT:
		case MEH_INPUT_BUTTON_UP:
			if (data->selected_executable == 0) {
				data->selected_executable = data->executables_length-1;
			} else {
				data->selected_executable -= 1;
			}
			meh_audio_play(app->audio, SFX_BIP);
			meh_exec_list_after_cursor_move(app, screen, prev_selected_exec);
			break;
		case MEH_INPUT_BUTTON_RIGHT:
		case MEH_INPUT_BUTTON_DOWN:
			if (data->selected_executable == data->executables_length-1) {
				data->selected_executable = 0;
			} else {
				data->selected_executable += 1;
			}
			meh_audio_play(app->audio, SFX_BIP);
			meh_exec_list_after_cursor_move(app, screen, prev_selected_exec);
			break;
		case MEH_INPUT_BUTTON_L:
			{
				int page = (data->selected_executable / (MEH_EXEC_LIST_SIZE)) - 1;
				data->selected_executable = page * MEH_EXEC_LIST_SIZE;
				if (data->selected_executable < 0) {
					data->selected_executable = data->executables_length-1;
				}
				meh_audio_play(app->audio, SFX_BIP);
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
				meh_audio_play(app->audio, SFX_BIP);
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

	if (g_strcmp0(data->platform->type, "complete") == 0) {
		/* update the executable description */
		meh_exec_desc_update(screen);
	} else {
		meh_cover_selec_update(screen);
	}

	return 0;
}

/*
 * meh_exec_list_resolve_tex resolves all the tex index
 * to real textures for object referencing them.
 */
static void meh_exec_list_resolve_tex(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	if (data->background > -1 && data->background_widget != NULL) {
		meh_widget_image_set_texture(data->background_widget, g_hash_table_lookup(data->textures, &(data->background)), FALSE);
	}
	if (data->cover > -1 && data->cover_widget != NULL) {
		meh_widget_image_set_texture(data->cover_widget, g_hash_table_lookup(data->textures, &(data->cover)), TRUE);
	}
	if (data->logo > -1 && data->logo_widget != NULL) {
		meh_widget_image_set_texture(data->logo_widget, g_hash_table_lookup(data->textures, &(data->logo)), TRUE);
	}
	for (int i = 0; i < 3; i++) {
		if (data->screenshots[i] > -1) {
			if (data->screenshots_widget[i] != NULL) {
				meh_widget_image_set_texture(data->screenshots_widget[i], g_hash_table_lookup(data->textures, &(data->screenshots[i])), TRUE);
			}
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

	/* background */
	if (data->background > -1) {
		meh_widget_image_render(app->window, data->background_widget);
	}

	/* background hover */
	meh_widget_rect_render(app->window, data->bg_hover_widget);

	/* header */
	meh_widget_text_render(app->window, data->header_text_widget);

	if (g_strcmp0(data->platform->type, "complete") == 0) {
		if (data->show_descriptions) {
			meh_exec_desc_render(app, screen);
		}
		meh_complete_selec_render(app, screen);
	} else {
		meh_cover_selec_render(app, screen);
	}

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
	return 0;
}
