#include <glib-2.0/glib.h>

#include "system/app.h"
#include "system/consts.h"
#include "view/screen.h"
#include "view/screen/debug_input.h"
#include "view/window.h"

static SDL_Color green = { 0, 255, 0, 255 };
static SDL_Color red = { 255, 0, 0, 255 };
static SDL_Color blue = { 0, 0, 255, 255 };

Screen* meh_screen_debug_input_new(App* app) {
	g_assert(app != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Debug input screen");
	screen->messages_handler = &meh_screen_debug_input_messages_handler;
	screen->destroy_data = &meh_screen_debug_input_destroy_data;

	/*
	 * Custom data
	 */
	DebugInputData* data = g_new(DebugInputData, 1);

	data->states = g_new(WidgetRect*, 10);

	// up, down, left, right
	data->states[0] = meh_widget_rect_new(100 +  0, 200 - 50, 50, 50, green, FALSE);
	data->states[1] = meh_widget_rect_new(100 +  0, 200 + 50, 50, 50, green, FALSE);
	data->states[2] = meh_widget_rect_new(100 - 50, 200 +  0, 50, 50, green, FALSE);
	data->states[3] = meh_widget_rect_new(100 + 50, 200 +  0, 50, 50, green, FALSE);

	data->states[4] = meh_widget_rect_new(600, 150, 50, 50, green, FALSE);
	data->states[5] = meh_widget_rect_new(520, 200, 50, 50, green, FALSE);

	data->states[6] = meh_widget_rect_new(100, 30, 100, 30, green, FALSE);
	data->states[7] = meh_widget_rect_new(500, 30, 100, 30, green, FALSE);

	data->states[8] = meh_widget_rect_new(370, 250, 70, 30, green, FALSE);
	data->states[9] = meh_widget_rect_new(270, 250, 70, 30, green, FALSE);

	screen->data = data;
	return screen;
}

void meh_screen_debug_input_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	DebugInputData* data = meh_screen_debug_input_get_data(screen);
	if (data != NULL) {
		for (int i = 0; i < 10; i++) {
			meh_widget_rect_destroy(data->states[i]);
			data->states[i] = NULL;
		}

		g_free(data->states);
		data->states = NULL;

		g_free(data);
	}
	screen->data = NULL;
}

DebugInputData* meh_screen_debug_input_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (DebugInputData*) screen->data;
}

int meh_screen_debug_input_messages_handler(struct App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				if (data->button == MEH_INPUT_SPECIAL_ESCAPE) {
					app->mainloop.running = FALSE;	
				}
			}
			break;
		case MEH_MSG_UPDATE:
			meh_screen_debug_input_update(app, screen);
			break;
		case MEH_MSG_RENDER:
			meh_screen_debug_input_render(app, screen, TRUE);
			break;
		}


	return 0;
}

/*
 * meh_screen_debug_input_update received a call by the main_loop when we 
 * can update this screen.
 */
void meh_screen_debug_input_update(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	DebugInputData* data = meh_screen_debug_input_get_data(screen);

	for (unsigned int i = 0; i < g_queue_get_length(app->input_manager->input_states); i++) {
		InputState* is = g_queue_peek_nth(app->input_manager->input_states, i);

		for (int j = 0; j < 10; j++) {
			switch (is->buttons_state[j]) {
				case MEH_INPUT_HOLD:
					data->states[j]->r.value = blue.r;
					data->states[j]->g.value = blue.g;
					data->states[j]->b.value = blue.b;
					data->states[j]->a.value = blue.a;
					break;
				case MEH_INPUT_JUST_PRESSED: // should never been displayed
					data->states[j]->r.value = green.r;
					data->states[j]->g.value = green.g;
					data->states[j]->b.value = green.b;
					data->states[j]->a.value = green.a;
					break;
				case MEH_INPUT_NOT_PRESSED:
					data->states[j]->r.value = red.r;
					data->states[j]->g.value = red.g;
					data->states[j]->b.value = red.b;
					data->states[j]->a.value = red.a;
					break;
			}
		}
	}
}

/*
 * meh_screen_debug_input_render is the rendering of the starting screen.
 */
void meh_screen_debug_input_render(App* app, Screen* screen, gboolean flip) {
	g_assert(screen != NULL);
	g_assert(app != NULL);

	DebugInputData* data = meh_screen_debug_input_get_data(screen);
	g_assert(data != NULL);

	/* clear the screen */
	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	for (int i = 0; i < 10; i++) {
		meh_widget_rect_render(app->window, data->states[i]);
	}

	meh_window_render(app->window);
}
