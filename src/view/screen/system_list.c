#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/input.h"
#include "system/message.h"
#include "view/screen.h"
#include "view/screen/system_list.h"

Screen* meh_screen_system_list_new(App* app) {
	Screen* screen = meh_screen_new();

	screen->name = g_strdup("System list screen");
	screen->messages_handler = &meh_screen_system_list_messages_handler;
	screen->destroy_data = &meh_screen_system_list_destroy_data;

	/* read the platforms in DB */
	SystemListData* data = g_new(SystemListData, 1);	
	data->platforms = meh_db_get_platforms(app->db);
	screen->data = data;

	return screen;
}

/*
 * meh_screen_system_list_destroy_data role is to delete the typed data of the screen
 */
void meh_screen_system_list_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	SystemListData* data = meh_screen_system_list_get_data(screen);
	if (data != NULL) {
		g_slist_free(data->platforms);
	}
	g_free(screen->data);
}

/*
 * meh_screen_system_list_data returns the data of the system_list screen
 */
SystemListData* meh_screen_system_list_get_data(Screen* screen) {
	SystemListData* data = (SystemListData*) screen->data;
	return data;
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

static void meh_screen_system_list_start_platform(App* app) {
	const gchar* working_dir = "/usr/bin";
	gchar* argv[] = { "xterm",
					  NULL };
	int exit_status = 0;
	GError* error = NULL;
	g_spawn_sync(working_dir,
				 argv,
				 NULL,
				 G_SPAWN_DEFAULT,
				 NULL,
				 NULL,
				 NULL,
				 NULL,
				 &exit_status,
				 &error);
	/* when launching something, we may have missed some
	 * input events, reset everything in case of. */
	meh_input_manager_reset_buttons_state(app->input_manager);
	/* FIXME When returning from the other app, if the user presses the same
	 * FIXME key as the one used to start the system, it will considered it
	 * FIXME as a repeatition. We should maybe generate a fake KEYUP event ?*/
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
				meh_screen_destroy(screen);
			} else {
				app->mainloop.running = FALSE;
			}
			break;

		case MEH_INPUT_BUTTON_UP:
			g_message("%d systems", g_slist_length(meh_screen_system_list_get_data(screen)->platforms));
			break;
		case MEH_INPUT_BUTTON_START:
			meh_screen_system_list_start_platform(app);
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
