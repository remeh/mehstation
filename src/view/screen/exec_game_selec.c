/*
 * mehstation - Executable selection in the executables list.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "view/screen/exec_game_selec.h"
#include "view/screen/executable_list.h"

void meh_game_selec_create_widgets(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color white_transparent = { 255, 255, 255, 50 };

	/* Selection */
	data->selection_widget = meh_widget_rect_new(0, -100, 415, 28, white_transparent, TRUE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 130, 500);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/* Executables */
	for (int i = 0; i < MEH_EXEC_LIST_SIZE; i++) {
		WidgetText* text = meh_widget_text_new(app->small_font, "", 55, 130+(i*32), 350, 30, white, FALSE);
		text->uppercase = TRUE; /* executables name in uppercase */
		g_queue_push_tail(data->executable_widgets, text);
	}
}

void meh_game_selec_destroy(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* Selection */
	meh_widget_rect_destroy(data->selection_widget);

	/* Executables */
	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_destroy( g_queue_peek_nth( data->executable_widgets, i) );
	}
	g_queue_free(data->executable_widgets);
}

void meh_game_selec_render(App* app, Screen* screen) {
	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* selection */
	meh_widget_rect_render(app->window, data->selection_widget);

	/* render all the executables names. */
	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_render(app->window, g_queue_peek_nth(data->executable_widgets, i));
	}
}

/*
 * meh_exec_list_refresh_executables_widget re-creates all the texture
 * in the text widgets for the executables.
 */
void meh_game_selec_refresh_executables_widgets(App* app, Screen* screen) {
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

