#pragma once

#include "system/audio.h"
#include "system/db.h"
#include "system/flags.h"
#include "system/input.h"
#include "system/message.h"
#include "system/settings.h"
#include "system/db/models.h"
#include "view/text.h"
#include "view/screen.h"
#include "view/window.h"

typedef struct {
	SDL_Event event;
	gboolean running;

	guint32 fps;
	guint32 framecount;   /* how many frame during the current cycle */
	guint32 frame_next_s; /* at which time we must compute the fps */

	/* making it public to allow the system to change it after
	 * having resumed mehstation when coming back from a platform */
	unsigned int next_tick;
} Mainloop;

typedef struct App {
	Window* window;
	Screen* current_screen;
	Font* small_font;
	Font* small_bold_font;
	Font* big_font;
	DB* db;
	Flags flags; /* cli params */
	Audio* audio; /* audio engine */
	InputManager* input_manager;
	Settings settings;
	Mainloop mainloop;

	gchar* conf_path; /* mehstation file configuration path */
	gchar* db_path;
	gchar* res_dir; /* resources directory */
} App;

App* meh_app_create();
int meh_app_init(App* app, int argc, char* argv[]);
void meh_app_exit(App* app);
int meh_app_destroy(App* app);
void meh_app_set_current_screen(App* app, Screen* screen, gboolean end_transitions);
int meh_app_main_loop(App* app);
void meh_app_main_loop_event(App* app);
void meh_app_main_loop_update(App* app);
void meh_app_main_loop_render(App* app);
void meh_app_send_message(App* app, Message* message);
gboolean meh_app_open_or_create_db(App*  app);
void meh_app_start_executable(App* app, Platform* platform, Executable* executable);
void meh_app_run_first_screen(App* app);
gchar* meh_app_init_create_dir_conf(App* app);
