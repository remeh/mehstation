/*
 * mehstation - Executable description in the executables list.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "view/screen.h"
#include "view/screen/executable_list.h"

static void meh_exec_desc_metadata_init(App* app, Screen* screen,
										WidgetText** label, WidgetText** value, gchar* title, int delay,
										int title_x, int title_y, int title_w, int title_h,
										int value_x, int value_y, int value_w, int value_h) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SDL_Color white = { 255, 255, 255, 255 };
	*label = meh_widget_text_new(app->small_bold_font, title, title_x, title_y, title_w, title_h, white, TRUE);
	*value = meh_widget_text_new(app->small_font, NULL, value_x, value_y, value_w, value_h, white, TRUE);
	(*value)->x = meh_transition_start(MEH_TRANSITION_CUBIC, value_x+MEH_FAKE_WIDTH, value_x, 300+delay);
	meh_screen_add_text_transitions(screen, (*value));
}

/*
 * meh_exec_desc_create_widgets creates the widget needed for the executable description.
 */
void meh_exec_desc_create_widgets(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	SDL_Color white = { 255, 255, 255, 0 };

	/*
	 * Extra information
	 */

	/* Genres */
	meh_exec_desc_metadata_init(app, screen,
								&data->genres_l_widget, &data->genres_widget,
								"Genres", 0,
								870, 396, 80, 30,
								1070, 400, 150, 30);

	/* Players */
	meh_exec_desc_metadata_init(app, screen,
								&data->players_l_widget, &data->players_widget,
								"Players", 0,
								870, 426, 80, 30,
								1070, 430, 150, 30);

	/* Publisher */
	meh_exec_desc_metadata_init(app, screen,
								&data->publisher_l_widget, &data->publisher_widget,
								"Publisher", 150,
								870, 456, 120, 30,
								1070, 460, 150, 30);

	/* Developer */
	meh_exec_desc_metadata_init(app, screen,
								&data->developer_l_widget, &data->developer_widget,
								"Developer", 150,
								870, 488, 150, 30,
								1070, 490, 150, 30);

	/* Rating */
	meh_exec_desc_metadata_init(app, screen,
								&data->rating_l_widget, &data->rating_widget,
								"Rating", 300,
								870, 518, 100, 30,
								1070, 520, 150, 30);

	/* Release date */
	meh_exec_desc_metadata_init(app, screen,
								&data->release_date_l_widget, &data->release_date_widget,
								 "Release date", 300,
								 870, 548, 150, 30,
								 1070, 550, 150, 30);

	/* Cover */
	data->cover_widget = meh_widget_image_new(NULL, 1030, 60, 200, 300);

	/* Logo */
	data->logo_widget = meh_widget_image_new(NULL, 530, 60, 350, 100);

	/* Screenshots */
	for (int i = 0; i < 3; i++) {
		data->screenshots_widget[i] = meh_widget_image_new(NULL, 500 + (265*i), 620, 190, 80);
	}

	/* Description */
	data->description_widget = meh_widget_text_new(app->small_font, NULL, 500, 180, 450, 280, white, FALSE);
	data->description_widget->multi = TRUE;
}

/*
 * meh_exec_desc_destroy releases the memory of the executable description.
 */
void meh_exec_desc_destroy(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen); if (data != NULL) {
		if (data->cover_widget != NULL) {
			meh_widget_image_destroy(data->cover_widget);
			data->cover_widget = NULL;
		}

		if (data->logo_widget != NULL) {
			meh_widget_image_destroy(data->logo_widget);
			data->logo_widget = NULL;
		}

		for (int i = 0; i < 3; i++) {
			meh_widget_image_destroy(data->screenshots_widget[i]);
		}

		meh_widget_text_destroy(data->genres_l_widget);
		meh_widget_text_destroy(data->genres_widget);
		meh_widget_text_destroy(data->players_l_widget);
		meh_widget_text_destroy(data->players_widget);
		meh_widget_text_destroy(data->publisher_l_widget);
		meh_widget_text_destroy(data->publisher_widget);
		meh_widget_text_destroy(data->developer_l_widget);
		meh_widget_text_destroy(data->developer_widget);
		meh_widget_text_destroy(data->rating_l_widget);
		meh_widget_text_destroy(data->rating_widget);
		meh_widget_text_destroy(data->release_date_l_widget);
		meh_widget_text_destroy(data->release_date_widget);

		meh_widget_text_destroy(data->description_widget);
	}
}

/*
 * meh_exec_desc_adapt_view adapts the view to the currently
 * selected executable.
 */
void meh_exec_desc_adapt_view(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	/*
	 * look whether the image is a portrait / landscape image
	 * and change the size of the description / cover in function
	 */

	ExecutableListData* data = meh_exec_list_get_data(screen);

	if (data->logo == -1) {
		/* no logo, use the full height for the description */
		data->description_widget->y.value = 50;
		data->description_widget->h = 280;
	} else {
		/* we have a logo, reduce the descrpition size and animate the logo */
		data->description_widget->y.value = 180;
		data->description_widget->h = 150;
		data->logo_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 60, 200);
		meh_screen_add_image_transitions(screen, data->logo_widget);
	}

	if (data->cover == -1 || data->cover_widget->texture == NULL) {
		/* no cover, use the full width for the description */
		data->description_widget->w = 650;
		data->cover_widget->texture = NULL;
	} else {
		/* detect the landscape/portrait mode */
		int w = 0,h = 0;
		SDL_QueryTexture(data->cover_widget->texture, NULL, NULL, &w, &h);
		if (w >= h) {
			/* landscape */
			data->cover_widget->x.value = 930;
			data->cover_widget->w.value = 300;
			data->cover_widget->h.value = 200;
			data->logo_widget->w.value = 340;
			data->description_widget->w = 340;
		} else {
			/* portrait */
			data->cover_widget->x.value = 1030;
			data->cover_widget->w.value = 200;
			data->cover_widget->h.value = 300;
			data->logo_widget->w.value = 440;
			data->description_widget->w = 440;
		}
	}

	/* 
	 * refreshes the text widgets about game info.
	 */

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);
	if (current_executable != NULL) {
		meh_widget_text_set_text(app->window, data->genres_widget, current_executable->genres);
		meh_widget_text_set_text(app->window, data->players_widget, current_executable->players);
		meh_widget_text_set_text(app->window, data->publisher_widget, current_executable->publisher);
		meh_widget_text_set_text(app->window, data->developer_widget, current_executable->developer);
		meh_widget_text_set_text(app->window, data->rating_widget, current_executable->rating);
		meh_widget_text_set_text(app->window, data->release_date_widget, current_executable->release_date);
		meh_widget_text_set_text(app->window, data->description_widget, current_executable->description);
	}

	/* re-creates the video widget overlay */
	
	if (data->exec_list_video != NULL) {
		meh_exec_list_video_destroy(data->exec_list_video);
		data->exec_list_video = NULL;
	}
	data->exec_list_video = meh_exec_list_video_new(app->window, screen, current_executable);

	/* if no video, we'll put the metadata instead. */
	if (!meh_exec_list_video_has_video(data->exec_list_video)) {
		data->genres_l_widget->x = data->players_l_widget->x =
		data->publisher_l_widget->x = data->developer_l_widget->x =
		data->rating_l_widget->x = data->release_date_l_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_l_widget->x.value, 500, 200);
		data->genres_widget->x = data->players_widget->x =
		data->publisher_widget->x = data->developer_widget->x =
		data->rating_widget->x = data->release_date_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_widget->x.value, 800, 200);
	} else {
		data->genres_l_widget->x = data->players_l_widget->x =
		data->publisher_l_widget->x = data->developer_l_widget->x =
		data->rating_l_widget->x = data->release_date_l_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_l_widget->x.value, 870, 200);
		data->genres_widget->x = data->players_widget->x =
		data->publisher_widget->x = data->developer_widget->x =
		data->rating_widget->x = data->release_date_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, data->genres_widget->x.value, 1070, 200);
	}

	/* starts the transition of the metadata */
	meh_screen_add_text_transitions(screen, data->genres_widget);
	meh_screen_add_text_transitions(screen, data->genres_l_widget);
	meh_screen_add_text_transitions(screen, data->players_widget);
	meh_screen_add_text_transitions(screen, data->players_l_widget);
	meh_screen_add_text_transitions(screen, data->publisher_widget);
	meh_screen_add_text_transitions(screen, data->publisher_l_widget);
	meh_screen_add_text_transitions(screen, data->developer_widget);
	meh_screen_add_text_transitions(screen, data->developer_l_widget);
	meh_screen_add_text_transitions(screen, data->rating_widget);
	meh_screen_add_text_transitions(screen, data->rating_l_widget);
	meh_screen_add_text_transitions(screen, data->release_date_widget);
	meh_screen_add_text_transitions(screen, data->release_date_l_widget);
}

/*
 * meh_exec_desc_update updates the widgets of the description part of the executable.
 */
void meh_exec_desc_update(Screen* screen) {
	g_assert(screen != NULL);

	ExecutableListData* data = meh_exec_list_get_data(screen);

	meh_widget_text_update(screen, data->description_widget);

	meh_widget_text_update(screen, data->genres_widget);
	meh_widget_text_update(screen, data->rating_widget);
	meh_widget_text_update(screen, data->publisher_widget);
	meh_widget_text_update(screen, data->release_date_widget);
	meh_widget_text_update(screen, data->developer_widget);
	meh_widget_text_update(screen, data->players_widget);

	meh_exec_list_video_update(screen, data->exec_list_video);
}

/*
 * meh_exec_desc_render renders the description part of the executable.
 */
void meh_exec_desc_render(App* app, Screen* screen) {

	ExecutableListData* data = meh_exec_list_get_data(screen);

	Executable* current_executable = g_queue_peek_nth(data->executables, data->selected_executable);

	/* cover */
	if (data->cover != -1) {
		meh_widget_image_render(app->window, data->cover_widget);
	}

	/* logo */
	if (data->logo != -1) {
		meh_widget_image_render(app->window, data->logo_widget);
	}

	/* render the screenshots */
	for (int i = 0; i < 3; i++) {
		meh_widget_image_render(app->window, data->screenshots_widget[i]);
	}

	/*
	 * extra info
	 */
	if (current_executable != NULL) {
		/* Genres */
		meh_widget_text_render(app->window, data->genres_l_widget);
		meh_widget_text_render(app->window, data->genres_widget);
		/* Players */
		meh_widget_text_render(app->window, data->players_l_widget);
		meh_widget_text_render(app->window, data->players_widget);
		/* Publisher */
		meh_widget_text_render(app->window, data->publisher_l_widget);
		meh_widget_text_render(app->window, data->publisher_widget);
		/* Developer */
		meh_widget_text_render(app->window, data->developer_l_widget);
		meh_widget_text_render(app->window, data->developer_widget);
		/* Release date */
		meh_widget_text_render(app->window, data->rating_l_widget);
		meh_widget_text_render(app->window, data->rating_widget);
		/* Release date */
		meh_widget_text_render(app->window, data->release_date_l_widget);
		meh_widget_text_render(app->window, data->release_date_widget);
		/* Description */
		meh_widget_text_render(app->window, data->description_widget);
	}

	/* video overlay */
	meh_exec_list_video_render(app->window, data->exec_list_video);
}
