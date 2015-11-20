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
	data->cover_widget = meh_widget_image_new(NULL, 120, 160, 400, 400);

	/* Logo */
	data->logo_widget = meh_widget_image_new(NULL, 600, 180, 600, 100);

	/* Description */
	data->description_widget = meh_widget_text_new(app->small_font, NULL, 650, 320, 490, 280, white, FALSE);
	data->description_widget->multi = TRUE;

	/* Place the next / prev executable labels
	 * NOTE(remy): the next_executable_widget is repositioned each time it is generated
	 * because we don't know this actual width and its necesarry to display aligned right. */

	data->prev_executable_widget = meh_widget_text_new(app->small_font, NULL, 20, MEH_FAKE_HEIGHT-50, 490, 280, white, FALSE);
	data->next_executable_widget = meh_widget_text_new(app->small_font, NULL, MEH_FAKE_WIDTH-100, MEH_FAKE_HEIGHT-50, 490, 280, white, FALSE);
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

	/* Description */
	if (data->description_widget != NULL) {
		meh_widget_text_destroy(data->description_widget);
		data->description_widget = NULL;
	}

	/* Next / prev widget */
	if (data->next_executable_widget != NULL) {
		meh_widget_text_destroy(data->next_executable_widget);
		data->next_executable_widget = NULL;
	}

	if (data->prev_executable_widget != NULL) {
		meh_widget_text_destroy(data->prev_executable_widget);
		data->prev_executable_widget = NULL;
	}
}

/* meh_cover_selec_adapt_view adapts the view (text, positions, ...) each time
 * the selected executable change. */
void meh_cover_selec_adapt_view(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (current_executable != NULL) {
		/* description */
		data->description_widget->text = current_executable->description;
		meh_widget_text_reload(app->window, data->description_widget);

		/* pick the prev / next executable and render their label */
		// TODO(remy): generation + position of next_executable_widget

		if (data->logo != -1) {
			data->logo_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 180, 200);
			meh_screen_add_image_transitions(screen, data->logo_widget);
		}
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

	if (data->logo != -1) {
		meh_widget_image_render(app->window, data->logo_widget);
	}

	meh_widget_text_render(app->window, data->description_widget);

	meh_widget_text_render(app->window, data->prev_executable_widget);
	meh_widget_text_render(app->window, data->next_executable_widget);
}
