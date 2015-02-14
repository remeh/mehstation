#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/message.h"
#include "view/screen.h"
#include "view/screen/system_list.h"

Screen* meh_screen_system_list_new() {
	Screen* screen = g_new(Screen, 1);

	screen->name = g_strdup("System list screen");
	screen->messages_handler = &meh_screen_system_list_messages_handler;
	screen->update = &meh_screen_system_list_update;

	return screen;
}

int meh_screen_system_list_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}

	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			meh_screen_system_list_button_pressed(app, screen, message);
			break;
	}

	return 0;
}

/*
 * meh_screen_system_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_system_list_button_pressed(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return;
	}

	int* data = (int*)message->data;
	g_message("Key pressed: %d\n", *data);
	switch (*data) {
		/* Return to the front page */
		case MEH_INPUT_SPECIAL_ESCAPE:
			/* Switch the current_screen to the parent screen */
			app->current_screen = screen->parent_screen;
			g_free(screen);
			break;
	}
}

int meh_screen_system_list_update(Screen* screen, int delta_time) {
	g_message("Update %s\n", screen->name);
	return 0;
}
