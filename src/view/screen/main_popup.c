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

static void meh_main_popup_button_pressed(App* app, Screen* screen, int pressed_button);
static void meh_main_popup_close(Screen* screen);

Screen* meh_main_popup_new(App* app, Screen* src_screen)  {
	g_assert(app != NULL);
	g_assert(src_screen != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Main popup screen");
	screen->messages_handler = &meh_main_popup_messages_handler;
	screen->destroy_data = &meh_main_popup_destroy_data;

	/*
	 * Custom data
	 */
	MainPopupData* data = g_new(MainPopupData, 1);

	data->src_screen = src_screen;
	data->action = 0;
	data->width = 400;
	data->height = 200;
	data->x = MEH_FAKE_WIDTH/2 - data->width/2;
	data->y = MEH_FAKE_HEIGHT/2 - data->height/2;
	data->quitting = FALSE;

	/* Popup background */

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color black = { 0, 0, 0, 240 };
	SDL_Color gray = { 15, 15, 15, 120 };
	SDL_Color light_gray = { 90, 90, 90, 220 };
	SDL_Color very_light_gray = { 40, 40, 40, 220 };

	data->background_widget = meh_widget_rect_new(data->x, data->y, data->width, data->height, very_light_gray, TRUE);
	data->background_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, -data->height, data->background_widget->y.value, 150);
	meh_screen_add_rect_transitions(screen, data->background_widget);
	screen->data = data;

	black.a = 150;
	data->hover_widget = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, black, TRUE); 

	/* Title */
	data->title_widget = meh_widget_text_new(app->small_bold_font, "COMMANDS", data->x+10, data->y+5, data->width-10, 40, white, TRUE);
	data->title_bg_widget = meh_widget_rect_new(data->x, data->y, data->width, 45, gray, TRUE);

	data->selection_widget = meh_widget_rect_new(
			data->x+5,
			data->y+54,
			data->width-10,
			30,
			light_gray,
			TRUE);


	/* Run random executable */
	data->random_widget = meh_widget_text_new(
			app->small_font,
			"Run random executable",
			data->x+20,
			data->y+55,
			data->width-20,
			30,
			white,
			TRUE);


	/* Shutdown */
	data->favorite_widget = meh_widget_text_new(
			app->small_font,
			"Shutdown",
			data->x+20,
			data->y+87,
			data->width-20,
			30,
			white,
			TRUE);
	screen->data = data;

	return screen;
}

/*
 * meh_main_popup_destroy_data destroys the additional data
 * of the popup screen.
 */
void meh_main_popup_destroy_data(Screen* screen) {
	MainPopupData* data = meh_main_popup_get_data(screen);
	meh_widget_rect_destroy(data->background_widget);
	meh_widget_rect_destroy(data->hover_widget);
	meh_widget_text_destroy(data->title_widget);
	meh_widget_rect_destroy(data->title_bg_widget);
	meh_widget_text_destroy(data->favorite_widget);
	meh_widget_rect_destroy(data->selection_widget);
	screen->data = NULL;
}

static void meh_main_popup_close(Screen* screen) {
	g_assert(screen != NULL);

	MainPopupData* data = meh_main_popup_get_data(screen);
	data->background_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, data->background_widget->y.value, MEH_FAKE_HEIGHT, 150);
	meh_screen_add_rect_transitions(screen, data->background_widget);
	data->quitting = TRUE;
}

MainPopupData* meh_main_popup_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (MainPopupData*) screen->data;
}

int meh_main_popup_messages_handler(struct App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_main_popup_button_pressed(app, screen, data->button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_main_popup_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_main_popup_render(app, screen);
			}
			break;
	}

	return 0;
}


/*
 * meh_main_popup_button_pressed moves the currently selected position.
 */
static void meh_main_popup_move_selection(App* app, Screen* screen) {
	g_assert(app);
	g_assert(screen);

	MainPopupData* data = meh_main_popup_get_data(screen);
	g_assert(data);

	int old = data->action;
	data->action = data->action ? 0 : 1;
	
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_LINEAR, (data->y+54) + old*32, (data->y+54) + (data->action*32), 100);
	meh_screen_add_rect_transitions(screen, data->selection_widget);
}

/*
 * meh_main_popup_button_pressed is called when we received a button pressed
 * message.
 */
static void meh_main_popup_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	MainPopupData* data = meh_main_popup_get_data(screen);
	g_assert(data);

	switch (pressed_button) {
		case MEH_INPUT_BUTTON_DOWN:
		case MEH_INPUT_BUTTON_UP:
			meh_main_popup_move_selection(app, screen);
			break;
		case MEH_INPUT_BUTTON_A:
			switch (data->action) {
				case 0:
					meh_main_popup_random_executable(app);
					break;
				case 1:
					meh_os_shutdown();
					break;
			}
		/* quit the popup */
		case MEH_INPUT_BUTTON_START:
		case MEH_INPUT_BUTTON_B:
		case MEH_INPUT_SPECIAL_ESCAPE:
			meh_main_popup_close(screen);
			break;
	}
}

/*
 * meh_main_popup_random_executable starts a random executable.
 */
void meh_main_popup_random_executable(struct App* app) {
	g_assert(app != NULL);

	int platform_id = 0;
	Executable* executable = meh_db_get_random_executable(app->db, &platform_id);
	if (executable) {
		Platform* platform = meh_db_get_platform(app->db, platform_id);
		if (platform) {
			meh_app_start_executable(app, platform, executable);
			meh_model_platform_destroy(platform);
		}
		meh_model_executable_destroy(executable);
	}
}

/*
 * meh_main_popup_update udpates the content of the main popup screen.
 */
int meh_main_popup_update(struct App* app, Screen* screen) {
	/* Animate the fading rect. */
	meh_screen_update_transitions(screen);

	MainPopupData* data = meh_main_popup_get_data(screen);

	/* update the src screen */
	meh_message_send(app, data->src_screen, MEH_MSG_UPDATE, NULL);

	/* quit the screen at the end of the exit animation. */
	if (data->quitting && data->background_widget->y.ended) {
		meh_app_set_current_screen(app, data->src_screen, TRUE);
		meh_screen_destroy(screen);
	}

	return 0;
}

void meh_main_popup_render(struct App* app, Screen* screen) {
	MainPopupData* data = meh_main_popup_get_data(screen);
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
		meh_widget_text_render(app->window, data->random_widget);
	}

	meh_window_render(app->window);
}
