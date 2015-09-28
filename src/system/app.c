/*
 * mehstation - App.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <SDL2/SDL_image.h>
#include <glib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "view/image.h"
#include "view/video.h"
#include "view/screen.h"
#include "view/screen/starting.h"
#include "system/app.h"
#include "system/consts.h"
#include "system/flags.h"
#include "system/input.h"
#include "system/message.h"
#include "system/settings.h"
#include "system/transition.h"
#include "system/db/models.h"

static void meh_settings_print_system_infos();

App* meh_app_create() {
	return g_new(App, 1);
}

int meh_app_init(App* app, int argc, char* argv[]) {
	g_assert(app != NULL);

	/* reads the flags */
	app->flags = meh_flags_parse(argc, argv);

	/* Nearly everything is used in the SDL. */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		g_critical("Can't init the SDL: %s", SDL_GetError());
		return 1;
	}

	if (TTF_Init() == -1) {
		g_critical("TTF can't initialize: %s\n", TTF_GetError());
		return 1;
	}

	if ( !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ) {
		g_critical("SDL_image can't initialize: %s\n", TTF_GetError());
		return 1;
	}

	g_message("SDL init OK.");

	/* ffmpeg */

	g_debug("Registering the ffmpeg codecs.");
	av_register_all();

	/* disable the screensaver. */
	SDL_DisableScreenSaver();

	meh_settings_print_system_infos();

	/* Init and read the settings */
	Settings settings;
	settings.fullscreen = FALSE;
	settings.zoom_logo = FALSE;
	meh_settings_read(&settings, "mehstation.conf");
	app->settings = settings;

	/* Open the DB */
	DB* db;
	db = meh_db_open_or_create("database.db");
	app->db = db;
	if (db == NULL) {
		return 2;
	}

	GQueue* platforms = meh_db_get_platforms(db);
	for (unsigned int i = 0; i <  g_queue_get_length(platforms); i++) {
		Platform* platform = g_queue_peek_nth(platforms, i);
		g_message("Platform %s found.", platform->name);
	}
	meh_model_platforms_destroy(platforms);


	/* Open the main window */
	Window* window = meh_window_create(settings.width, settings.height, settings.fullscreen, app->flags.force_software);
	app->window = window;

	/* Opens some font. */

	Font* font = meh_font_open("res/fonts/OpenSans-Regular.ttf", meh_window_convert_width(window, 18));
	app->small_font = font;
	font = meh_font_open("res/fonts/OpenSans-Bold.ttf", meh_window_convert_width(window, 22));
	app->small_bold_font = font;
	font = meh_font_open("res/fonts/OpenSans-Bold.ttf", meh_window_convert_width(window, 36));
	app->big_font = font;

	/* Input manager */
	InputManager* input_manager = meh_input_manager_new(app->db, settings);
	app->input_manager = input_manager;

	/* Sets the starting screen as the current screen */
	Screen* starting_screen = meh_screen_starting_new(app);
	if (starting_screen == NULL) {
		g_critical("Can't init the starting screen.");
		return 1;
	} 

	/* Hides the mouse cursor */
	SDL_ShowCursor(SDL_DISABLE);

	meh_app_set_current_screen(app, starting_screen, FALSE);

	return 0;
}

/*
 * meh_app_exit is called just before freeing all the resources.
 */
void meh_app_exit(App* app) {
	g_assert(app != NULL);
	g_message("mehstation is closing.");
}

/*
 * meh_app_destroy frees the resources of the given app
 * and closes every system (SDL, ...)
 */
int meh_app_destroy(App* app) {
	g_assert(app != NULL);

	meh_db_close(app->db);

	/* Free the resource */
	meh_font_destroy(app->small_font);
	app->small_font = NULL;
	meh_font_destroy(app->small_bold_font);
	app->small_bold_font = NULL;
	meh_font_destroy(app->big_font);
	app->big_font = NULL;

	meh_window_destroy(app->window);
	app->window = NULL;

	if (app->current_screen != NULL) {
		meh_screen_destroy(app->current_screen);
	}

	meh_input_manager_destroy(app->input_manager);

	SDL_Quit();
	TTF_Quit();

	return 0;
}

void meh_app_set_current_screen(App* app, Screen* screen, gboolean end_transitions) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	g_message("Setting the current screen to : %s", screen->name);
	if (app->current_screen != NULL && end_transitions) {
	  meh_transitions_end(app->current_screen->transitions); /* before leaving this screen, we must end all its transitions. */
	}
	app->current_screen = screen;
}

/*
 * meh_app_main_loop is the main loop running for the whole time 
 * of the process.
 * Simple main loop with constant update speed and max rendering.
 */
int meh_app_main_loop(App* app) {
	g_assert(app != NULL);

	//int last_time = SDL_GetTicks();
	app->mainloop.running = TRUE;

	/* application lifecycle */
	int loop_count = 0;
	app->mainloop.next_tick = SDL_GetTicks();
	int start_tick = SDL_GetTicks();

	const int MAX_FRAMESKIP = app->settings.max_frameskip;
	const int DELTA_TO_SKIP = 1000 / app->settings.max_update_per_second;

	while (app->mainloop.running) {
		loop_count = 0;
		start_tick = SDL_GetTicks();

		while(SDL_GetTicks() > app->mainloop.next_tick && loop_count < MAX_FRAMESKIP) {
			meh_app_main_loop_event(app);
			meh_app_main_loop_update(app);
			app->mainloop.next_tick += DELTA_TO_SKIP;
			loop_count++;
		}

		meh_app_main_loop_render(app);

		/* fps count */
		guint32 now = SDL_GetTicks();
		app->mainloop.framecount++;
		if (now > app->mainloop.frame_next_s) {
			app->mainloop.fps = app->mainloop.framecount;
			g_debug("%d fps", app->mainloop.fps);
			app->mainloop.frame_next_s = now + 1000;
			app->mainloop.framecount = 0;
		}

		/* well, don't want to use 100% of CPU and mehstation
		 * won't need so much resources to render...
		 * Look how long we can sleep.
		 * We can sleep 16 - delta if the delta < 16ms (60fps)
		 * we even remove some seconds for the OS */
		int delta = SDL_GetTicks() - start_tick;
		if (delta < 16) {
			delta -= 3;
			if (delta < 0) { delta = 0; }
			SDL_Delay(16-delta);
		}
	}

	return 0;
}

/*
 * meh_app_main_loop_event is the event handling part
 * of the pipeline.
 */
void meh_app_main_loop_event(App* app) {
	g_assert(app != NULL);

	SDL_Event* event = &(app->mainloop.event);
	while (SDL_PollEvent(event)) {
		switch (event->type) {
			case SDL_JOYDEVICEADDED:
				meh_input_manager_destroy(app->input_manager);
				app->input_manager = meh_input_manager_new(app->db, app->settings);
				break;
			case SDL_JOYDEVICEREMOVED:
				meh_input_manager_destroy(app->input_manager);
				app->input_manager = meh_input_manager_new(app->db, app->settings);
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				meh_input_manager_read_event(app->input_manager, event);
				break;
			case SDL_JOYBUTTONUP:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYAXISMOTION:
				meh_input_manager_read_event(app->input_manager, event);
				break;
			case SDL_QUIT:
				/* directly stop the app */
				app->mainloop.running = FALSE;
				break;
		}
	}

	/* Generate events from input */
	GSList* list_messages = meh_input_manager_generate_messages(app->input_manager);

	for (unsigned int i = 0; i < g_slist_length(list_messages); i++) {
		/* create the Message */
		Message* message = g_slist_nth_data(list_messages, i);
		/* send it to the current screen */
		meh_app_send_message(app, message);
		/* release the message */
		meh_input_message_destroy(message);
	}

	/* release the memory of the list */
	g_slist_free(list_messages);
}

void meh_app_main_loop_update(App* app) {
	g_assert(app != NULL);

	/* sends the update message */
	Message* message = meh_message_new(MEH_MSG_UPDATE, NULL);
	meh_app_send_message(app, message);
	meh_message_destroy(message);
	message = NULL;
}

/*
 * meh_app_main_loop_render is the rendering part of the pipeline.
 */
void meh_app_main_loop_render(App* app) {
	g_assert(app != NULL);

	/* sends the render message */
	Message* message = meh_message_new(MEH_MSG_RENDER, NULL);
	meh_app_send_message(app, message);
	meh_message_destroy(message);
	message = NULL;
}

void meh_app_send_message(App* app, Message* message) {
	g_assert(app != NULL);

	if (message == NULL) {
		g_warning("NULL message has been seen in meh_app_send_message");
		return;
	}

	/* route the message to the screen. */
	if (app->current_screen != NULL) {
		app->current_screen->messages_handler(app, app->current_screen, message);
	}
}

/*
 * meh_app_start_executable starts the given platform's executable.
 * FIXME the algorithm to replace the flags could be better
 * (and not using a queue)
 */
void meh_app_start_executable(App* app, Platform* platform, Executable* executable) {
	g_assert(platform != NULL);
	g_assert(executable != NULL);

	if (platform->command == NULL || executable->filepath == NULL ||
		strlen(platform->command) == 0 || strlen(executable->filepath) == 0) {
		g_critical("Platform command or executable filepath empty.");
		return;
	}

	/* prepare the exec call */
	/* FIXME path with spaces ? */
	gchar** command_parts = g_strsplit(platform->command, " ", -1);
	GQueue* commands = g_queue_new();

	/* build a queue with all the args and replace in the 
	 * same time the %exec% and %extra% flags */
	int i = 0;
	while (command_parts[i] != NULL) {
		if (g_strcmp0(command_parts[i], "\%exec\%") == 0) {
			/* replace by the value */
			g_queue_push_tail(commands, g_strdup(executable->filepath));
		} else if (g_strcmp0(command_parts[i], "\%extra\%") == 0) {
			if (executable->extra_parameter != NULL && strlen(executable->extra_parameter) > 0) {
				/* replace by the value */
				g_queue_push_tail(commands, g_strdup(executable->extra_parameter));
			}
		} else {
			g_queue_push_tail(commands, g_strdup(command_parts[i]));
		}
		i++;
	}

	/* release used memory */
	g_strfreev(command_parts);

	/* rebuild an array of strings free the queue
	 * NOTE that we don't release the memory of the strings
	 * because they are reused in the array */
	gchar** parts = g_new(gchar*, g_queue_get_length(commands)+1);
	for (int i = 0; i < g_queue_get_length(commands); i++) {
		parts[i] = g_queue_peek_nth(commands, i);
	}
	parts[g_queue_get_length(commands)] = NULL;
	g_queue_free(commands);

	g_debug("Launching '%s' on '%s'", executable->display_name, platform->name);

	int exit_status = 0;
	GError* error = NULL;
	g_spawn_sync(NULL,
				 parts,
				 NULL,
				 G_SPAWN_DEFAULT,
				 NULL,
				 NULL,
				 NULL,
				 NULL,
				 &exit_status,
				 &error);

	if (error != NULL) {
		g_critical("can't start the platform '%s' with the executable '%s' with command '%s'.", platform->name, executable->display_name, platform->command);
		g_critical("%s", error->message);
		g_error_free(error);
	}

	g_debug("End of execution of '%s'", executable->display_name);

	/* when launching something, we may have missed some
	 * input events, reset everything in case of. */
	meh_input_manager_reset_buttons_state(app->input_manager);
	/* FIXME When returning from the other app, if the user presses the same
	 * FIXME key as the one used to start the system, SDL will considered it
	 * FIXME as a repeatition. We should maybe generate a fake KEYUP event ?*/
	SDL_RaiseWindow(app->window->sdl_window);

	/* reset the next_tick to avoid extra update/rendering frames. */
	app->mainloop.next_tick = SDL_GetTicks();
}

/*
 * Display some system information on the standart output.
 * Unused.
 */
static void meh_settings_print_system_infos() {
	SDL_DisplayMode display_mode;
	int i;

	g_message("Available displays: %d", SDL_GetNumVideoDisplays());
	for (i = 0; i < SDL_GetNumVideoDisplays(); i++) {
		if (SDL_GetCurrentDisplayMode(i, &display_mode) == 0) {
			g_message("Display #%d: display mode is %dx%d@%dhz.", i, display_mode.w, display_mode.h, display_mode.refresh_rate);
		}
	}

	g_message("Video driver: %s", SDL_GetCurrentVideoDriver());
}
