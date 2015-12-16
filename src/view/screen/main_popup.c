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
#include "view/screen/simple_popup.h"
#include "view/screen/platform_list.h"

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

	SimplePopupData* data = meh_simple_popup_get_data(screen);

	int platform_id = 0;
	Executable* executable = meh_db_get_random_executable(app->db, &platform_id);
	if (executable) {
		Platform* platform = meh_db_get_platform(app->db, platform_id);
		if (platform) {
			/* destroy the platform screen */
			PlatformListData* platform_list_data = meh_screen_platform_list_get_data(data->src_screen);
			int selected_platform = platform_list_data->selected_platform;
			meh_screen_destroy(data->src_screen);
			data->src_screen = NULL;
			platform_list_data = NULL;

			meh_app_start_executable(app, platform, executable);

			meh_model_platform_destroy(platform);

			/* recreate the platform screen */
			data->src_screen = meh_screen_platform_list_new(app);
			platform_list_data = meh_screen_platform_list_get_data(data->src_screen);
			platform_list_data->selected_platform = selected_platform;

			meh_screen_platform_change_platform(app, data->src_screen);
		}
		meh_model_executable_destroy(executable);
	}
}
