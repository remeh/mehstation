/*
 * mehstation - Reusable popup.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/consts.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/simple_popup.h"
#include "view/screen/executable_list.h"
#include "view/screen/exec_selection.h"

static void meh_simple_popup_button_pressed(App* app, Screen* screen, int pressed_button);
static void meh_simple_popup_close(App* app, Screen* screen);

Screen* meh_simple_popup_new(App* app, Screen* src_screen, Platform* platform, Executable* executable)  {
	g_assert(app != NULL);
	g_assert(src_screen != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Popup screen");
	screen->messages_handler = &meh_simple_popup_messages_handler;
	screen->destroy_data = &meh_simple_popup_destroy_data;

	/*
	 * Custom data
	 */
	SimplePopupData* data = g_new(SimplePopupData, 1);

	data->executable = NULL;
	if (executable) {
		data->executable = executable;
	}
	data->platform = NULL;
	if (platform) {
		data->platform = platform;
	}

	data->src_screen = src_screen;
	data->action = 0;
	data->width = 400;
	data->height = 200;
	data->x = MEH_FAKE_WIDTH/2 - data->width/2;
	data->y = MEH_FAKE_HEIGHT/2 - data->height/2;

	/* Popup list of actions */
	data->actions = g_queue_new();

	/* Popup background */

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color black = { 0, 0, 0, 240 };
	SDL_Color light_gray = { 40, 40, 40, 220 };

	screen->data = data;

	black.a = 210;
	data->hover_widget = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, black, TRUE); 

	/* Title */
	data->title_widget = meh_widget_text_new(app->small_bold_font, "OPTIONS", -300, data->y+5, data->width-10, 40, white, TRUE);
	data->title_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, -300, 300, 350);
	meh_screen_add_text_transitions(screen, data->title_widget);

	data->selection_widget = meh_widget_rect_new(
			0,
			data->y+54,
			MEH_FAKE_WIDTH,
			30,
			light_gray,
			TRUE);

	screen->data = data;

	return screen;
}

/*
 * meh_simple_popup_destroy_data destroys the additional data
 * of the popup screen.
 */
void meh_simple_popup_destroy_data(Screen* screen) {
	SimplePopupData* data = meh_simple_popup_get_data(screen);
	meh_widget_rect_destroy(data->hover_widget);
	meh_widget_text_destroy(data->title_widget);
	meh_widget_rect_destroy(data->selection_widget);

	/* free the actions objects */
	for (unsigned int i = 0; i < g_queue_get_length(data->actions); i++) {
		SimplePopupAction* action = g_queue_peek_nth(data->actions, i);
		meh_widget_text_destroy(action->widget);
		g_free(action->label);
	}
	g_queue_free(data->actions);

	screen->data = NULL;
}

SimplePopupData* meh_simple_popup_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (SimplePopupData*) screen->data;
}

/*
 * meh_simple_popup_update udpates the content of the popup screen.
 */
int meh_simple_popup_update(struct App* app, Screen* screen) {
	/* Animate the fading rect. */
	meh_screen_update_transitions(screen);

	SimplePopupData* data = meh_simple_popup_get_data(screen);

	/* update the src screen */
	meh_message_send(app, data->src_screen, MEH_MSG_UPDATE, NULL);

	return 0;
}
/*
 *
 * meh_popup_button_pressed moves the currently selected position.
 */
static void meh_simple_popup_move_selection(App* app, Screen* screen, gboolean down) {
	g_assert(app);
	g_assert(screen);

	SimplePopupData* data = meh_simple_popup_get_data(screen);
	g_assert(data);

	/* TODO(remy): support action change */
	int old = data->action;

	if (down) {
		data->action += 1;
	} else {
		data->action -= 1;
	}

	int actions_count = g_queue_get_length(data->actions);

	if (data->action < 0) {
		data->action = actions_count-1;
	} else if (data->action >= actions_count) {
		data->action = 0;
	}

	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, (data->y+54) + old*32, (data->y+54) + (data->action*32), 100);
	meh_screen_add_rect_transitions(screen, data->selection_widget);
}

/**
 * meh_simple_popup_run_action runs the underlying action.
 */
static void meh_simple_popup_run_action(App* app, Screen* screen, int selected_action) {
	SimplePopupData* data = meh_simple_popup_get_data(screen);
	SimplePopupAction* action = g_queue_peek_nth(data->actions, selected_action);
	if (action != NULL) {
		action->run(app, screen);
	}
}

/*
 * meh_simple_popup_add_action adds an action to the popup.
 */
void meh_simple_popup_add_action(App* app, Screen* screen, gchar* label, void (*func) (struct App*, struct Screen*)) {
	g_assert(label != NULL);
	g_assert(func != NULL);


	SimplePopupData* data = meh_simple_popup_get_data(screen);
	SimplePopupAction* new_action = g_new(SimplePopupAction, 1);

	new_action->label = label;
	new_action->run = func;

	SDL_Color white = { 255, 255, 255, 0 };

	/* compute the position of this widget */
	new_action->widget = meh_widget_text_new(
			app->small_font,
			new_action->label,
			400,
			data->y+55 + (32*g_queue_get_length(data->actions)),
			data->width-20,
			30,
			white,
			TRUE);

	g_queue_push_tail(data->actions, new_action);
}

/*
 * meh_simple_popup_button_pressed is called when we received a button pressed
 * message.
 */
void meh_simple_popup_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SimplePopupData* data = meh_simple_popup_get_data(screen);

	switch (pressed_button) {
		case MEH_INPUT_BUTTON_DOWN:
			meh_simple_popup_move_selection(app, screen, TRUE);
			break;
		case MEH_INPUT_BUTTON_UP:
			meh_simple_popup_move_selection(app, screen, FALSE);
			break;
		case MEH_INPUT_BUTTON_A:
			meh_simple_popup_run_action(app, screen, data->action);
			break;
		/* quit the popup */
		case MEH_INPUT_BUTTON_START:
		case MEH_INPUT_BUTTON_B:
		case MEH_INPUT_SPECIAL_ESCAPE:
			meh_simple_popup_close(app, screen);
			break;
	}
}

static void meh_simple_popup_close(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	SimplePopupData* data = meh_simple_popup_get_data(screen);

	meh_app_set_current_screen(app, data->src_screen, TRUE);
	meh_screen_destroy(screen);
	screen = NULL;
}

int meh_simple_popup_messages_handler(struct App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_simple_popup_button_pressed(app, screen, data->button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_simple_popup_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_simple_popup_render(app, screen);
			}
			break;
	}

	return 0;
}

void meh_simple_popup_render(struct App* app, Screen* screen) {
	SimplePopupData* data = meh_simple_popup_get_data(screen);
	g_assert(data != NULL);

	/* render the background screen */
	gboolean* flip = g_new(gboolean, 1);
	*flip = FALSE;
	meh_message_send(app, data->src_screen, MEH_MSG_RENDER, flip);

	/* render the popup screen */

	meh_widget_rect_render(app->window, data->hover_widget);

	/* title */
	meh_widget_text_render(app->window, data->title_widget);

	meh_widget_rect_render(app->window, data->selection_widget);

	/* render the action */
	for (int i = 0; i < g_queue_get_length(data->actions); i++) {
		SimplePopupAction* action = g_queue_peek_nth(data->actions, i);
		meh_widget_text_render(app->window, action->widget);
	}

	meh_window_render(app->window);
}

