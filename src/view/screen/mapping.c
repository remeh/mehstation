/*
 * mehstation - Configure input.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/input.h"
#include "system/message.h"
#include "system/db.h"
#include "system/db/models.h"
#include "view/image.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/fade.h"
#include "view/screen/mapping.h"
#include "view/screen/platform_list.h"
#include "view/window.h"

Screen* meh_screen_mapping_new(App* app) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Gamepad configuration screen");
	screen->messages_handler = &meh_screen_mapping_messages_handler;
	screen->destroy_data = &meh_screen_mapping_destroy_data;

	/*
	 * Custom data
	 */
	MappingData* data = g_new(MappingData, 1);

	SDL_Color white = { 255, 255, 255, 255 };

	data->title = meh_widget_text_new(app->big_font, "Press a key on the controller to configure", 50, 50, 1230, 50, white, FALSE);
	data->title->x = meh_transition_start(MEH_TRANSITION_LINEAR, -300, 50, 300);
	meh_screen_add_text_transitions(screen, data->title);
	data->device_configuring = NULL;
	data->action = NULL;

	data->step = MEH_MAPPING_STEP_IDENTIFY;

	screen->data = data;

	return screen;
}

int meh_screen_mapping_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_screen_mapping_button_pressed(app, screen, data->button, data->sdl_key, data->guid);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_mapping_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			if (message->data == NULL) {
				meh_screen_mapping_render(app, screen, TRUE);
			} else {
				gboolean* flip = (gboolean*)message->data;
				meh_screen_mapping_render(app, screen, *flip);
			}
			break;
	}

	return 0;
}

void meh_screen_mapping_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	MappingData* data = meh_screen_mapping_get_data(screen);

	meh_widget_text_destroy(data->title);

	if (data->device_configuring != NULL) {
		meh_widget_text_destroy(data->device_configuring);
	}

	if (data->action != NULL) {
		meh_widget_text_destroy(data->action);
	}

	screen->data = NULL;
}

/*
 * meh_screen_mapping_get_data returns the specific data of the
 * screen
 */
MappingData* meh_screen_mapping_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (MappingData*) screen->data;
}

static void meh_screen_mapping_next_screen(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);


	MappingData* data = meh_screen_mapping_get_data(screen);
	g_assert(data != NULL);

	/* build the mapping object */
	Mapping* mapping = meh_model_mapping_new(data->device_configuring->text, data->up, data->down, data->left, data->right,
												data->start, data->select, data->a, data->b, data->l, data->r);

	/* delete an eventual mapping with the same id */
	meh_db_delete_mapping(app->db, data->device_configuring->text);

	/* save the created mapping */
	meh_db_save_mapping(app->db, mapping);

	/* re-assign the mapping */
	meh_input_manager_assign_mapping(app->db, app->input_manager);

	meh_model_mapping_destroy(mapping);
	mapping = NULL;

	/* create the child screen */
	Screen* platform_screen = meh_screen_platform_list_new(app);
	Screen* fade_screen = meh_screen_fade_new(app, screen, platform_screen);
	meh_app_set_current_screen(app, fade_screen, TRUE);
	/* NOTE we don't free the memory of the starting screen, the fade screen
	 * will do it. */
}

/*
 * meh_screen_mapping_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_mapping_button_pressed(App* app, Screen* screen, int pressed_button, int sdl_key, gchar*  guid) {
	MappingData* data = meh_screen_mapping_get_data(screen);
	g_assert(data != NULL);

	SDL_Color white = { 255, 255, 255, 255 };

	Gamepad* gamepad = meh_input_manager_gamepad_by_guid(app->input_manager, guid);
	const char* name = "keyboard";
	if (gamepad != NULL) {
		name = gamepad->name;
	}

	switch (data->step) {
		case MEH_MAPPING_STEP_IDENTIFY:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->title);
			data->title = meh_widget_text_new(app->big_font, "Configuring:", 50, 50, 1230, 50, white, FALSE);
			data->device_configuring = meh_widget_text_new(app->big_font, name, 300, 50, 1030, 50, white, FALSE);
			data->action = meh_widget_text_new(app->big_font, "Press the key for up", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			break;
		case MEH_MAPPING_STEP_UP:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for down", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);

			data->up = sdl_key;
			break;
		case MEH_MAPPING_STEP_DOWN:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for left", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->down = sdl_key;
			break;
		case MEH_MAPPING_STEP_LEFT:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for right", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->left = sdl_key;
			break;
		case MEH_MAPPING_STEP_RIGHT:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for A button", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->right = sdl_key;
			break;
		case MEH_MAPPING_STEP_A:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for B button", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->a = sdl_key;
			break;
		case MEH_MAPPING_STEP_B:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for L button", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->b = sdl_key;
			break;
		case MEH_MAPPING_STEP_L:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for R button", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->l = sdl_key;
			break;
		case MEH_MAPPING_STEP_R:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for START button", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->r = sdl_key;
			break;
		case MEH_MAPPING_STEP_START:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "Press the key for SELECT button", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->start = sdl_key;
			break;
		case MEH_MAPPING_STEP_SELECT:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_widget_text_destroy(data->action);
			data->action = meh_widget_text_new(app->big_font, "All done", 200, 150, 1030, 50, white, FALSE);
			data->action->x = meh_transition_start(MEH_TRANSITION_LINEAR, MEH_FAKE_WIDTH+200, 200, 200);
			meh_screen_add_text_transitions(screen, data->action);
			data->select = sdl_key;
			break;
		case MEH_MAPPING_STEP_END:
			meh_transitions_end(screen->transitions); meh_screen_update_transitions(screen);
			meh_screen_mapping_next_screen(app, screen);

			break;
	}

	meh_input_manager_reset_buttons_state(app->input_manager);

	data->step++;
}

/*
 * meh_screen_mapping_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_mapping_update(App* app, Screen* screen) {
	/* update the transition of the current screen */
	meh_screen_update_transitions(screen);

	return 0;
}

/*
 * meh_screen_mapping_render is the rendering of the starting screen.
 */
void meh_screen_mapping_render(App* app, Screen* screen, gboolean flip) {
	g_assert(screen != NULL);
	g_assert(app != NULL);

	MappingData* data = meh_screen_mapping_get_data(screen);
	g_assert(data != NULL);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	meh_widget_text_render(app->window, data->title);

	if (data->device_configuring != NULL) {
		meh_widget_text_render(app->window, data->device_configuring);
	}
	if (data->action != NULL) {
		meh_widget_text_render(app->window, data->action);
	}

	if (flip == TRUE) {
		meh_window_render(app->window);
	}
}
