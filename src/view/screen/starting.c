#include "glib-2.0/glib.h"

#include "system/app.h"
#include "system/consts.h"
#include "system/event.h"
#include "view/screen.h"
#include "view/screen/starting.h"

Screen* meh_screen_starting_new() {
	Screen* screen = g_new(Screen, 1);

	screen->name = g_strdup("Starting screen");
	screen->events_handler = &meh_screen_starting_events_handler;
	screen->update = &meh_screen_starting_update;

	return screen;
}

int meh_screen_starting_events_handler(App* app, Screen* screen, Event* event) {
	g_assert(screen != NULL);

	if (event == NULL) {
		return 0;
	}

	switch (event->id) {
		case MEH_EVENT_ESCAPE:
			app->mainloop.running = FALSE;
			break;
	}

	return 0;
}

/*
 * meh_screen_starting_update received a call by the main_loop when we 
 * can update this screen.
 */
int meh_screen_starting_update(struct Screen* screen, int delta_time) {
	return 0;
}
