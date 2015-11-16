/*
 * mehstation - Executable selection in the executables list.
 *
 * NOTE(remy): rename to cover_selec ?
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "view/screen/exec_app_selec.h"
#include "view/screen/executable_list.h"

void meh_game_app_create_widgets(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color white_transparent = { 255, 255, 255, 50 };

	/* Selection */
	data->selection_widget = meh_widget_rect_new(0, -100, 415, 28, white_transparent, TRUE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 130, 500);
	meh_screen_add_rect_transitions(screen, data->selection_widget);
}
