/*
 * mehstation - Executable selection in the executables list.
 *
 * NOTE(remy): rename to cover_selec ?
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/transition.h"
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

	data->prev_executable_widget = meh_widget_text_new(app->small_bold_font, NULL, 20, MEH_FAKE_HEIGHT-50, (MEH_FAKE_WIDTH/2) - 40, 280, white, TRUE);
	data->next_executable_widget = meh_widget_text_new(app->small_bold_font, NULL, MEH_FAKE_WIDTH-100, MEH_FAKE_HEIGHT-50, (MEH_FAKE_WIDTH/2) - 40, 200, white, TRUE);

	data->prev_arrow_exec_widget = meh_widget_text_new(app->small_bold_font, "<", 20, MEH_FAKE_HEIGHT-50, 20, 30, white, TRUE);
	data->next_arrow_exec_widget = meh_widget_text_new(app->small_bold_font, ">", MEH_FAKE_WIDTH-30, MEH_FAKE_HEIGHT-50, 20, 30, white, TRUE);
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

	/* Logo */
	if (data->logo_widget != NULL) {
		meh_widget_image_destroy(data->logo_widget);
		data->logo_widget = NULL;
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

	if (data->next_arrow_exec_widget != NULL) {
		meh_widget_text_destroy(data->next_arrow_exec_widget);
		data->next_arrow_exec_widget = NULL;
	}

	if (data->prev_arrow_exec_widget != NULL) {
		meh_widget_text_destroy(data->prev_arrow_exec_widget);
		data->prev_arrow_exec_widget = NULL;
	}

	if (data->prev_executable_widget != NULL) {
		meh_widget_text_destroy(data->prev_executable_widget);
		data->prev_executable_widget = NULL;
	}

	/* Executable title */
	if (data->executable_title_widget != NULL) {
		meh_widget_text_destroy(data->executable_title_widget);
		data->executable_title_widget = NULL;
	}
}

void meh_cover_selec_update(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	if (data->prev_executable_widget != NULL) {
		meh_widget_text_update(screen, data->prev_executable_widget);
	}

	if (data->next_executable_widget != NULL) {
		meh_widget_text_update(screen, data->next_executable_widget);
	}
}

/* meh_cover_selec_adapt_view adapts the view (text, positions, ...) each time
 * the selected executable change. */
void meh_cover_selec_adapt_view(App* app, Screen* screen, int prev_selected_id) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	int next_exec_id = data->selected_executable + 1;
	int prev_exec_id = data->selected_executable - 1;

	if (prev_exec_id < 0) {
		prev_exec_id = g_queue_get_length(data->executables) - 1;
	}
	if (next_exec_id >= g_queue_get_length(data->executables)) {
		next_exec_id = 0;
	}

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	Executable* prev_executable = g_queue_peek_nth(data->executables, prev_exec_id);
	Executable* next_executable = g_queue_peek_nth(data->executables, next_exec_id);

	if (current_executable != NULL) {
		/* description */
		data->description_widget->text = current_executable->description;
		meh_widget_text_reload(app->window, data->description_widget);

		/* pick the prev / next executable and render their label */
		if (prev_executable != NULL && prev_executable->display_name != NULL) {
			gchar* upper = g_utf8_strup(prev_executable->display_name, -1);
			meh_widget_text_set_text(app->window, data->prev_executable_widget, upper);
			g_free(upper);
		}

		if (next_executable != NULL && next_executable->display_name != NULL) {
			gchar* upper = g_utf8_strup(next_executable->display_name, -1);
			meh_widget_text_set_text(app->window, data->next_executable_widget, upper);
			g_free(upper);
		}

		if (data->cover != -1 && data->cover_widget->texture != NULL) {
			/* detect the landscape/portrait mode */
			int w = 0,h = 0;
			SDL_QueryTexture(data->cover_widget->texture, NULL, NULL, &w, &h);
			if (w >= h) {
				/* landscape */
				data->cover_widget->x.value = 80;
				data->cover_widget->y.value = 180;
				data->cover_widget->w.value = 480;
				data->cover_widget->h.value = 360;
			} else {
				/* portrait */
				data->cover_widget->x.value = 160;
				data->cover_widget->y.value = 120;
				data->cover_widget->w.value = 320;
				data->cover_widget->h.value = 480;
			}
		}

		/* animate prev/next executables text */
		if (prev_selected_id > data->selected_executable) {
			// direction: previous
			int right_pos = MEH_FAKE_WIDTH-40-data->next_executable_widget->tex_w;
			if (right_pos < ((MEH_FAKE_WIDTH/2))) {
				right_pos = (MEH_FAKE_WIDTH/2);
			}

			data->next_executable_widget->x = meh_transition_start(
					MEH_TRANSITION_CUBIC,
					data->prev_executable_widget->x.value,
					right_pos,
					300);
			meh_screen_add_text_transitions(screen, data->next_executable_widget);

			data->prev_executable_widget->x = meh_transition_start(
					MEH_TRANSITION_CUBIC,
					-200,
					40,
					300);
			meh_screen_add_text_transitions(screen, data->prev_executable_widget);
		} else {
			// direction: next
			int right_pos = MEH_FAKE_WIDTH-40-data->next_executable_widget->tex_w;
			if (right_pos < ((MEH_FAKE_WIDTH/2))) {
				right_pos = (MEH_FAKE_WIDTH/2);
			}

			data->prev_executable_widget->x = meh_transition_start(
					MEH_TRANSITION_CUBIC,
					data->next_executable_widget->x.value,
					40,
					300);
			meh_screen_add_text_transitions(screen, data->prev_executable_widget);

			data->next_executable_widget->x = meh_transition_start(
					MEH_TRANSITION_CUBIC,
					MEH_FAKE_WIDTH+200, 
					right_pos,
					300);
			meh_screen_add_text_transitions(screen, data->next_executable_widget);
		}

		if (data->logo != -1) {
			data->logo_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 180, 200);
			meh_screen_add_image_transitions(screen, data->logo_widget);
		} else {
			/* there is no logo, generates a title */
			if (data->executable_title_widget != NULL) {
				meh_widget_text_destroy(data->executable_title_widget);
			}
			SDL_Color white = { 255, 255, 255, 255 };
			data->executable_title_widget = meh_widget_text_new(app->big_font, current_executable->display_name, 650, 180, 600, 200, white, FALSE);
			data->executable_title_widget->multi = TRUE;
			meh_widget_text_reload(app->window, data->executable_title_widget);
			data->executable_title_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 180, 200);
			meh_screen_add_text_transitions(screen, data->executable_title_widget);
		}

		/* favorite ? */
		Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
		if (current_executable != NULL && current_executable->favorite) {
			data->favorite_widget->x.value = MEH_FAKE_WIDTH-100;
			data->favorite_widget->y.value = 100;
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
	} else if (data->executable_title_widget != NULL) {
		meh_widget_text_render(app->window, data->executable_title_widget);
	}

	/* favorite */
	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (current_executable != NULL && current_executable->favorite) {
		meh_widget_image_render(app->window, data->favorite_widget);
	}

	meh_widget_text_render(app->window, data->description_widget);

	meh_widget_text_render(app->window, data->prev_executable_widget);
	meh_widget_text_render(app->window, data->next_executable_widget);
	meh_widget_text_render(app->window, data->prev_arrow_exec_widget);
	meh_widget_text_render(app->window, data->next_arrow_exec_widget);
}
