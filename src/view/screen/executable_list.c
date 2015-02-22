#include <glib-2.0/glib.h>

#include "system/app.h"
#include "system/consts.h"
#include "system/db.h"
#include "system/input.h"
#include "system/message.h"
#include "system/db/models.h"
#include "view/image.h"
#include "view/screen.h"
#include "view/screen/executable_list.h"

static void meh_screen_exec_list_destroy_resources(Screen* screen);
static void meh_screen_exec_list_load_resources(App* app, Screen* screen);
static void meh_screen_exec_list_start_executable(App* app, Screen* screen);
static void meh_screen_exec_list_select_resources(Screen* screen);

Screen* meh_screen_exec_list_new(App* app, int platform_id) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new();

	screen->name = g_strdup("Executable list screen");
	screen->messages_handler = &meh_screen_exec_list_messages_handler;
	screen->destroy_data = &meh_screen_exec_list_destroy_data;

	/*
	 * init the custom data.
	 */
	ExecutableListData* data = g_new(ExecutableListData, 1);	
	/* get the platform */
	data->platform = meh_db_get_platform(app->db, platform_id);
	g_assert(data->platform != NULL);
	/* get the executables */
	data->executables = meh_db_get_platform_executables(app->db, data->platform, TRUE);
	data->executables_length = g_queue_get_length(data->executables);
	data->selected_executable = 0;
	data->textures = NULL;
	data->background = NULL;
	data->cover = NULL;
	screen->data = data;

	/* Load the first resources */
	meh_screen_exec_list_load_resources(app, screen);
	/* Select a background */
	meh_screen_exec_list_select_resources(screen);

	return screen;
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
	}

	/* We must free the textures cache */
	meh_screen_exec_list_destroy_resources(screen);

	g_free(screen->data);
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
			g_message("Freeing the texture id %d", *key);
			SDL_DestroyTexture(texture);
		}
	}

	g_list_free(keys);
	g_hash_table_destroy(data->textures);
}

/*
 * meh_screen_exec_list_select_background uses the resources of the currently selected
 * executable to select a background.
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
	 * Select a random resource if any
	 */
	int length = g_queue_get_length(executable->resources);
	if (length == 0) {
		return;
	}

	int rand = g_random_int_range(0, length);

	ExecutableResource* resource = g_queue_peek_nth(executable->resources, rand);
	if (resource == NULL) {
		return;
	}

	data->background = g_hash_table_lookup(data->textures, &(resource->id));
	g_message("Selected background : %d (%p)", resource->id, data->background);

	/*
	 * Select a cover
	 */
	int i = 0;
	for (i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* res = g_queue_peek_nth(executable->resources, i++);
		if (res != NULL) {
			if (g_strcmp0(res->type, "cover") == 0) {
				data->cover = g_hash_table_lookup(data->textures, &(res->id));
				g_message("Selected cover: %d", res->id);
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
				int* delta_time = (int*)message->data;
				meh_screen_exec_list_update(screen, *delta_time);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_screen_exec_list_render(app, screen);
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

		/* Look whether or not it's already in the cache. */
		if (g_hash_table_lookup(data->textures, &(resource->id)) != NULL) {
			g_message("Not reloading the %s ID %d", resource->type, resource->id);
			continue;
		}

		g_message("Loading the %s ID %d", resource->type, resource->id);
		SDL_Texture* texture = meh_image_load_file(app->window->sdl_renderer, resource->filepath);
		if (texture != NULL) {
			int* id = g_new(int, 1); *id = resource->id;
			g_hash_table_insert(data->textures, id, texture);
		}
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
}

/*
 * meh_screen_exec_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_exec_list_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);

	switch (pressed_button) {
		case MEH_INPUT_SPECIAL_ESCAPE:
			if (screen->parent_screen != NULL) {
				/* back to the platform screen */
				meh_app_set_current_screen(app, screen->parent_screen);
				meh_screen_destroy(screen);
			} else {
				/* exit mehstation */
				app->mainloop.running = FALSE;
			}
			break;
		case MEH_INPUT_BUTTON_UP:
			if (data->selected_executable == 0) {
				/* FIXME length on a the slist could be a bit slow (iterate over the whole list for the count) */
				data->selected_executable = data->executables_length-1;
			} else {
				data->selected_executable -= 1;
			}
			meh_screen_exec_list_load_resources(app, screen);
			meh_screen_exec_list_select_resources(screen);
			break;
		case MEH_INPUT_BUTTON_DOWN:
				/* FIXME length on a the slist could be a bit slow (iterate over the whole list for the count) */
			if (data->selected_executable == data->executables_length-1) {
				data->selected_executable = 0;
			} else {
				data->selected_executable += 1;
			}
			meh_screen_exec_list_load_resources(app, screen);
			meh_screen_exec_list_select_resources(screen);
			break;
		case MEH_INPUT_BUTTON_START:
			meh_screen_exec_list_start_executable(app, screen);
			break;
	}
}

int meh_screen_exec_list_update(Screen* screen, int delta_time) {
	g_assert(screen != NULL);

	return 0;
}

int meh_screen_exec_list_render(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	ExecutableListData* data = meh_screen_exec_list_get_data(screen);

	/* background */
	if (data->background != NULL) {
		SDL_Rect viewport = { 0, 0, app->window->width, app->window->height };
		SDL_SetTextureBlendMode(data->background, SDL_BLENDMODE_ADD);
		SDL_SetTextureAlphaMod(data->background, 40);
		meh_window_render_texture(app->window, data->background, viewport);
	}

	/* cover */
	if (data->cover != NULL) {
		SDL_Rect viewport = { 600, 200, 300, 400 };
		SDL_SetTextureBlendMode(data->background, SDL_BLENDMODE_NONE);
		meh_window_render_texture(app->window, data->cover, viewport);
	}

	SDL_Color white = { 255, 255, 255 };
	meh_window_render_text(app->window, app->small_font, "mehstation 1.0", white, 50, 50);
	meh_window_render_text(app->window, app->small_font, data->platform->name, white, 250, 100);

	int i = 0;
	for (i = 0; i < g_queue_get_length(data->executables); i++) {
		Executable* executable = g_queue_peek_nth(data->executables, i);
		meh_window_render_text(app->window, app->small_font, executable->display_name, white, 100, 130+(i*35));
	}

	/* cursor */
	meh_window_render_text(app->window, app->small_font, "->", white, 70, 130+(data->selected_executable*35));
	
	meh_window_render(app->window);
	SDL_Delay(10); /* TODO delta */
	return 0;
}

