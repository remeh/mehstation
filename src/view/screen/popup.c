/*
 * mehstation - Popup.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/consts.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/popup.h"
#include "view/screen/executable_list.h"

static void meh_screen_popup_favorite_toggle(App* app, Screen* screen);
static void meh_screen_popup_button_pressed(App* app, Screen* screen, int pressed_button);
static void meh_screen_popup_close(Screen* screen);

Screen* meh_screen_popup_new(App* app, Screen* src_screen, Executable* executable)  {
	g_assert(app != NULL);
	g_assert(src_screen != NULL);
	g_assert(executable != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Popup screen");
	screen->messages_handler = &meh_screen_popup_messages_handler;
	screen->destroy_data = &meh_screen_popup_destroy_data;

	/*
	 * Custom data
	 */
	PopupData* data = g_new(PopupData, 1);

	data->src_screen = src_screen;
	data->executable = executable;
	data->action = 0;
	data->width = 400;
	data->height = 200;
	data->x = MEH_FAKE_WIDTH/2 - data->width/2;
	data->y = MEH_FAKE_HEIGHT/2 - data->height/2;
	data->quitting = FALSE;

	/* Popup background */

	SDL_Color white = { 255, 255, 255 };
	SDL_Color black = { 0, 0, 0, 240 };
	SDL_Color gray = { 15, 15, 15, 220 };
	SDL_Color light_gray = { 80, 80, 80, 220 };

	data->background_widget = meh_widget_rect_new(data->x, data->y, data->width, data->height, black, TRUE); 
	data->background_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, -data->height, data->background_widget->y.value, 150);
	meh_screen_add_rect_transitions(screen, data->background_widget);
	screen->data = data;

	black.a = 100;
	data->hover_widget = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, black, TRUE); 
	screen->data = data;

	/* Title */
	data->title_widget = meh_widget_text_new(app->small_bold_font, "OPTIONS", data->x+10, data->y+5, data->width-10, 40, white, TRUE);
	data->title_bg_widget = meh_widget_rect_new(data->x, data->y, data->width, 45, gray, TRUE);

	data->selection_widget = meh_widget_rect_new(
			data->x+5,
			data->y+54,
			data->width-10,
			30,
			light_gray,
			TRUE);


	/* Add/Remove to favorite */
	data->favorite_widget = meh_widget_text_new(
			app->small_font,
			executable->favorite == 1 ? "Remove from favorite" : "Add to favorite",
			data->x+20,
			data->y+55,
			data->width-20,
			30,
			white,
			TRUE);

	return screen;
}

/*
 * meh_screen_popup_destroy_data destroys the additional data
 * of the popup screen.
 */
void meh_screen_popup_destroy_data(Screen* screen) {
	PopupData* data = meh_screen_popup_get_data(screen);
	meh_widget_rect_destroy(data->background_widget);
	meh_widget_rect_destroy(data->hover_widget);
	meh_widget_text_destroy(data->title_widget);
	meh_widget_rect_destroy(data->title_bg_widget);
	meh_widget_text_destroy(data->favorite_widget);
	meh_widget_rect_destroy(data->selection_widget);
	screen->data = NULL;
}

PopupData* meh_screen_popup_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (PopupData*) screen->data;
}

/*
 * meh_screen_popup_update udpates the content of the popup screen.
 */
int meh_screen_popup_update(struct App* app, Screen* screen) {
	/* Animate the fading rect. */
	meh_screen_update_transitions(screen);

	PopupData* data = meh_screen_popup_get_data(screen);

	/* update the src screen */
	meh_message_send(app, data->src_screen, MEH_MSG_UPDATE, NULL);

	/* quit the screen at the end of the exit animation. */
	if (data->quitting && data->background_widget->y.ended) {
		meh_app_set_current_screen(app, data->src_screen, TRUE);
		meh_screen_destroy(screen);
	}

	return 0;
}

/*
 * meh_screen_popup_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_popup_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PopupData* data = meh_screen_popup_get_data(screen);

	switch (pressed_button) {
		case MEH_INPUT_BUTTON_A:
			switch (data->action) {
				case 0:
					meh_screen_popup_favorite_toggle(app, screen);
					break;
			}
		/* quit the popup */
		case MEH_INPUT_BUTTON_B:
		case MEH_INPUT_SPECIAL_ESCAPE:
			meh_screen_popup_close(screen);
			break;
	}
}

static void meh_screen_popup_favorite_toggle(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PopupData* data = meh_screen_popup_get_data(screen);
	ExecutableListData* exec_list_data = meh_exec_list_get_data(data->src_screen);

	/* updates the value of the executable */

	gboolean new_value = data->executable->favorite == 1 ? FALSE : TRUE;

	if (meh_db_set_executable_favorite(app->db, data->executable, new_value)) {
		data->executable->favorite = new_value;
	}

	/* re-position the executable in the executables list */

	int prev_selected = exec_list_data->selected_executable;

	unsigned int i = 0;

	/* retrieves the one which will move in the list */
	Executable* to_move = g_queue_pop_nth(exec_list_data->executables, exec_list_data->selected_executable);

	for (i = 0; i < g_queue_get_length(exec_list_data->executables); i++) {
		Executable* ex = g_queue_peek_nth(exec_list_data->executables, i);
		/* if favorite, ensure to stay in the favorite zone */
		if (new_value == TRUE) {
			if (ex->favorite == FALSE)  {
				break;
			}
		}
		if (g_strcmp0(ex->display_name, data->executable->display_name) > 0) {
			if (new_value == TRUE && ex->favorite == TRUE) {
				break;
			}
			else if (new_value == FALSE && ex->favorite == FALSE) {
				break;
			}
		}
	}

	GList* after = g_queue_peek_nth_link(exec_list_data->executables, i);

	if ((new_value == TRUE || g_queue_get_length(exec_list_data->executables) <= 1)) {
		g_queue_insert_before(exec_list_data->executables, after, to_move);
	} else {
		g_queue_insert_before(exec_list_data->executables, after, to_move);
	}

	/* notify the screen of the new selected executable */

	exec_list_data->selected_executable = i;

	/* redraw the executables list texts */

	meh_exec_list_refresh_executables_widget(app, data->src_screen);

	/* move and redraw the selection */

	meh_exec_list_after_cursor_move(app, data->src_screen, prev_selected);

	/* finally close the popup */

	meh_screen_popup_close(screen);
}

static void meh_screen_popup_close(Screen* screen) {
	g_assert(screen != NULL);

	PopupData* data = meh_screen_popup_get_data(screen);
	data->background_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, data->background_widget->y.value, MEH_FAKE_HEIGHT, 150);
	meh_screen_add_rect_transitions(screen, data->background_widget);
	data->quitting = TRUE;
}

int meh_screen_popup_messages_handler(struct App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_screen_popup_button_pressed(app, screen, data->button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_popup_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_screen_popup_render(app, screen);
			}
			break;
	}

	return 0;
}

void meh_screen_popup_render(struct App* app, Screen* screen) {
	PopupData* data = meh_screen_popup_get_data(screen);
	g_assert(data != NULL);

	/* render the background screen */
	gboolean* flip = g_new(gboolean, 1);
	*flip = FALSE;
	meh_message_send(app, data->src_screen, MEH_MSG_RENDER, flip);


	/* render the popup screen */

	meh_widget_rect_render(app->window, data->hover_widget);
	meh_widget_rect_render(app->window, data->background_widget);

	if (data->background_widget->y.ended) {
		/* title */
		meh_widget_rect_render(app->window, data->title_bg_widget);
		meh_widget_text_render(app->window, data->title_widget);

		meh_widget_rect_render(app->window, data->selection_widget);
		meh_widget_text_render(app->window, data->favorite_widget);
	}

	meh_window_render(app->window);
}
