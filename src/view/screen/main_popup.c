/*
 * mehstation - Main popup.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/consts.h"
#include "system/os.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/main_popup.h"

/**
 * meh_main_popup_os_shutdown shutdowns the OS.
 */
void meh_main_popup_os_shutdown(App* app, Screen* screen) {
	meh_os_shutdown();
}

/**
 * meh_main_popup_close closes mehstation.
 */
void meh_main_popup_close(App* app, Screen* screen) {
	/* stop the main loop */
	app->mainloop.running = FALSE;
}

/*
 * meh_main_popup_random_executable starts a random executable.
 */
void meh_main_popup_random_executable(App* app, Screen* screen) {
	g_assert(app != NULL);

	int platform_id = 0;
	Executable* executable = meh_db_get_random_executable(app->db, &platform_id);
	if (executable) {
		Platform* platform = meh_db_get_platform(app->db, platform_id);
		if (platform) {
			meh_app_start_executable(app, platform, executable);
			meh_model_platform_destroy(platform);
		}
		meh_model_executable_destroy(executable);
	}
}
