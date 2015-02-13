#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/message.h"
#include "view/screen.h"
#include "view/screen/starting.h"

Screen* meh_screen_starting_new() {
	Screen* screen = g_new(Screen, 1);

	screen->name = g_strdup("Starting screen");
	screen->messages_handler = &meh_screen_starting_messages_handler;
	screen->update = &meh_screen_starting_update;

	return screen;
}

int meh_screen_starting_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			meh_screen_starting_button_pressed(app, screen, message);
			break;
	}

	return 0;
}

/*
 * meh_screen_starting_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_starting_button_pressed(App* app, Screen* screen, Message* message) {
	int* data = (int*)message->data;
	switch (*data) {
		/* Escape on the starting screen quit the app */
		case MEH_INPUT_SPECIAL_ESCAPE:
			app->mainloop.running = FALSE;
			break;
	}
}

/*
 * meh_screen_starting_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_starting_update(struct Screen* screen, int delta_time) {
	return 0;
}
