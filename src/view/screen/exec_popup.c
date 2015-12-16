/*
 * mehstation - Popup.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "view/screen.h"
#include "view/screen/exec_complete_selec.h"
#include "view/screen/executable_list.h"
#include "view/screen/platform_list.h"
#include "view/screen/simple_popup.h"

void meh_exec_popup_start_random_executable(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SimplePopupData* data = meh_simple_popup_get_data(screen);
	ExecutableListData* exec_list_data = meh_exec_list_get_data(data->src_screen);

	/* get a random executable */
	int platform_id = exec_list_data->platform->id;
	Executable* executable = meh_db_get_platform_random_executable(app->db, platform_id);

	if (executable) {
		Platform* platform = meh_db_get_platform(app->db, platform_id);
		if (platform) {
			/* keep some infos */
			PlatformListData* platform_list_data = meh_screen_platform_list_get_data(data->src_screen->parent_screen);

			int cursor_platform = platform_list_data->selected_platform;
			int cursor_exec = exec_list_data->selected_executable;

			meh_app_start_executable(app, platform, executable);

			/* destroy screens view */
			meh_screen_destroy(data->src_screen->parent_screen);
			data->src_screen->parent_screen = NULL;
			platform_list_data = NULL;
			meh_screen_destroy(data->src_screen);
			app->current_screen = NULL;
			exec_list_data = NULL;

			/* recreate the executable list view */
			data->src_screen = meh_exec_list_new(app, platform->id);
			exec_list_data = meh_exec_list_get_data(data->src_screen);

			/* restore the cursor position */
			exec_list_data->selected_executable = cursor_exec;
			meh_exec_list_after_cursor_move(app, data->src_screen, 0);

			/* recreate the parent screen which is the platform list */
			Screen* platform_screen = meh_screen_platform_list_new(app);
			data->src_screen->parent_screen = platform_screen;

			/* restore the cursor position */
			platform_list_data = meh_screen_platform_list_get_data(platform_screen);
			platform_list_data->selected_platform = cursor_platform;
			meh_screen_platform_change_platform(app, platform_screen);

			meh_model_platform_destroy(platform);
		}
		meh_model_executable_destroy(executable);
	}
}

void meh_exec_popup_favorite_toggle(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SimplePopupData* data = meh_simple_popup_get_data(screen);
	ExecutableListData* exec_list_data = meh_exec_list_get_data(data->src_screen);

	/* updates the value of the executable */

	gboolean new_value = data->executable->favorite == 1 ? FALSE : TRUE;

	if (meh_db_set_executable_favorite(app->db, data->executable, new_value)) {
		data->executable->favorite = new_value;
	}

	/* re-position the executable in the executables list if necessary */
	if (g_queue_get_length(exec_list_data->executables) > 1) {
		int prev_selected = exec_list_data->selected_executable;

		unsigned int i = 0;

		/* retrieves the one which will move in the list */
		Executable* to_move = g_queue_pop_nth(exec_list_data->executables, exec_list_data->selected_executable);

		/* find the good position for the moved executable */

		for (i = 0; i < g_queue_get_length(exec_list_data->executables); i++) {
			gboolean exit = FALSE;
			Executable* ex = g_queue_peek_nth(exec_list_data->executables, i);
			/* if favorite, ensure to stay in the favorite zone */
			if (new_value == TRUE) {
				if (ex->favorite == FALSE)  {
					exit = TRUE;
				}
			}

			gchar* first = g_utf8_strup(ex->display_name, g_utf8_strlen(ex->display_name, -1));
			gchar* second = g_utf8_strup(data->executable->display_name, g_utf8_strlen(ex->display_name, -1));

			if (g_utf8_collate(first, second) > 0) {
				if (new_value == TRUE && ex->favorite == TRUE) {
					exit = TRUE;
				}
				else if (new_value == FALSE && ex->favorite == FALSE) {
					exit = TRUE;
				}
			}

			g_free(first);
			g_free(second);

			if (exit) {
				break;
			}
		}

		GList* after = g_queue_peek_nth_link(exec_list_data->executables, i);

		/* re-add it to the good position */

		g_queue_insert_before(exec_list_data->executables, after, to_move);

		/* notify the screen of the new selected executable */

		exec_list_data->selected_executable = i;

		/* redraw the executables list texts */

		meh_complete_selec_refresh_executables_widgets(app, data->src_screen);

		/* move and redraw the selection */

		meh_exec_list_after_cursor_move(app, data->src_screen, prev_selected);
	}

	meh_app_set_current_screen(app, data->src_screen, TRUE);
}
