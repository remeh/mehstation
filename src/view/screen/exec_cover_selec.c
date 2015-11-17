/*
 * mehstation - Executable selection in the executables list.
 *
 * NOTE(remy): rename to cover_selec ?
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "view/screen/exec_cover_selec.h"
#include "view/screen/executable_list.h"

void meh_cover_selec_create_widgets(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color white_transparent = { 255, 255, 255, 50 };

	/* Selection */
	data->selection_widget = meh_widget_rect_new(0, -100, 415, 28, white_transparent, TRUE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 130, 500);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/* Cover */
	data->cover_widget = meh_widget_image_new(NULL, 440, 160, 400, 400);
}

void meh_cover_selec_destroy(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	/* Selection */
	if (data->selection_widget != NULL) {
		meh_widget_rect_destroy(data->selection_widget);
		data->selection_widget = NULL;
	}

	/* Cover */
	if (data->cover_widget != NULL) {
		meh_widget_image_destroy(data->cover_widget);
		data->cover_widget = NULL;
	}
}

void meh_cover_selec_render(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);
	
	/* cover */
	if (data->cover != -1) {
		meh_widget_image_render(app->window, data->cover_widget);
	}
}
